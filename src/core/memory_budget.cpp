#include "memory_budget.h"
#include "godot_cpp/classes/array_mesh.hpp"
#include "godot_cpp/classes/file_access.hpp"
#include "godot_cpp/classes/image.hpp"
#include "godot_cpp/classes/image_texture.hpp"
#include "godot_cpp/classes/object.hpp"
#include "godot_cpp/classes/ref.hpp"
#include "godot_cpp/classes/resource_loader.hpp"
#include "godot_cpp/core/defs.hpp"
#include "godot_cpp/core/mutex_lock.hpp"
#include "godot_cpp/core/object.hpp"
#include "godot_cpp/core/object_id.hpp"
#include "godot_cpp/templates/hash_set.hpp"

using namespace godot;

size_t MemoryBudget::_get_size(const Ref<Resource> &p_resource) const {
	//scuffed size estimation for resources.
	auto class_name = p_resource->get_class();

	if (class_name == "Image") {
		Ref<Image> image = p_resource;
		return image->get_data().size();
	}

	if (class_name == "ImageTexture") {
		Ref<ImageTexture> image_texture = p_resource;
		auto img = image_texture->get_image();
		if (img.is_valid()) {
			return img->get_data().size();
		}
	}

	if (class_name == "ArrayMesh") {
		size_t total = 0;
		Ref<ArrayMesh> array_mesh = p_resource;
		for (int i = 0; i < array_mesh->get_surface_count(); ++i) {
			auto arrays = array_mesh->surface_get_arrays(i);
			for (int j = 0; j < arrays.size(); j++) {
				if (arrays[j].get_type() == Variant::PACKED_BYTE_ARRAY) {
					total += PackedByteArray(arrays[j]).size();
				} else if (arrays[j].get_type() == Variant::PACKED_VECTOR3_ARRAY) {
					total += PackedVector3Array(arrays[j]).size() * sizeof(Vector3);
				} else if (arrays[j].get_type() == Variant::PACKED_VECTOR2_ARRAY) {
					total += PackedVector2Array(arrays[j]).size() * sizeof(Vector2);
				}
			}
		}
		return total;
	}

	const auto &path = p_resource->get_path();
	if (!path.is_empty() && FileAccess::file_exists(path)) {
		auto file = FileAccess::open(path, FileAccess::READ);
		if (file.is_valid()) {
			return file->get_length();
		}
	}

	return 0;
}

size_t MemoryBudget::_get_estimated_size(const String &p_path) const {
	if (p_path.is_empty() || !FileAccess::file_exists(p_path)) {
		return 1024 * 1024;
	}

	auto file = FileAccess::open(p_path, FileAccess::READ);
	if (!file.is_valid()) {
		return 1024 * 1024;
	}

	size_t file_size = file->get_length() * 0.1f;
	return file_size;
}

void MemoryBudget::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_mb", "type", "mb"), &MemoryBudget::set_budget);
	ClassDB::bind_method(D_METHOD("process_pending_resources"), &MemoryBudget::process_pending_resources);
}

MemoryBudget::MemoryBudget() {
	_cache_mutex.instantiate();
}

// this function will be called from separate worker threads.
void MemoryBudget::register_resource(const Ref<Resource> &p_resource) {
	if (!p_resource.is_valid()) {
		return;
	}

	auto id = ObjectID{ p_resource->get_instance_id() };
	auto type = p_resource->get_class();
	auto estimated_size = _get_estimated_size(p_resource->get_path());

	MutexLock lock(*_cache_mutex.ptr());

	if (_cache.has(ObjectID{ p_resource->get_instance_id() })) {
		return;
	}

	PendingEntry entry;
	entry.id = id;
	entry.type = type;
	entry.estimated = estimated_size;

	_pending_resources.push_back(entry);

	if (_estimated.has(type)) {
		_estimated[type] += estimated_size;
	} else {
		_estimated[type] = estimated_size;
	}
}

size_t MemoryBudget::bytes(const String &p_type) const {
	if (p_type.is_empty()) {
		size_t bytes = 0;
		for (const auto &type : _bytes) {
			bytes += type.value;
		}
		return bytes;
	}

	return _bytes[p_type];
}

size_t MemoryBudget::estimated(const String &p_type) const {
	if (p_type.is_empty()) {
		size_t estimated = 0;
		for (const auto &type : _estimated) {
			estimated += type.value;
		}
		return estimated;
	}

	return _estimated[p_type];
}

// this function will be run on the main thread
void MemoryBudget::process_pending_resources(const int p_max) {
	Vector<PendingEntry> to_process;

	{
		MutexLock lock(*_cache_mutex.ptr());

		auto count = MIN(_pending_resources.size(), p_max);

		to_process.resize(count);

		auto it = _pending_resources.begin();
		for (size_t i = 0; i < count; ++i, ++it) {
			to_process.write[i] = *it;
		}

		for (int i = count - 1; i >= 0; --i) {
			_pending_resources.remove_at(i);
		}
	}

	// out of lock cuz _get_size is heavy
	Vector<ObjectID> id_list;
	Vector<StringName> type_list;
	Vector<size_t> bytes_list;
	Vector<size_t> estimated_list;
	HashSet<int> pending_to_remove;
	for (const auto &entry : to_process) {
		auto obj = ObjectDB::get_instance(entry.id);
		Ref<Resource> resource = Object::cast_to<Resource>(obj);

		size_t size = 0;
		if (!obj || !resource.is_valid()) {
			pending_to_remove.insert(id_list.size());
		} else {
			size = _get_size(resource);
		}

		bytes_list.push_back(size);
		id_list.push_back(entry.id);
		type_list.push_back(entry.type);
		estimated_list.push_back(entry.estimated);
	}

	{
		MutexLock lock(*_cache_mutex.ptr());
		for (int i = 0; i < id_list.size(); ++i) {
			auto id = id_list[i];
			auto type = type_list[i];
			auto bytes = bytes_list[i];
			auto estimated = estimated_list[i];

			if (_estimated.has(type) && _estimated[type] >= estimated) {
				_estimated[type] -= estimated;
			}

			if (pending_to_remove.has(i)) {
				continue;
			}

			if (_bytes.has(type)) {
				_bytes[type] += bytes;
			} else {
				_bytes[type] = bytes;
			}

			_cache.insert(id, { bytes, type });
		}

		Vector<ObjectID> to_remove;
		for (auto it = _cache.begin(); it != _cache.end(); ++it) {
			Object *obj = ObjectDB::get_instance(it->key);
			if (!obj || obj->is_queued_for_deletion()) {
				to_remove.push_back(it->key);
			}
		}

		for (const auto &key : to_remove) {
			const auto &entry = _cache[key];
			_bytes[entry.type] -= entry.bytes;
			_cache.erase(key);
			//UtilityFunctions::print("ERASED OBJECT");
		}
	}
}

void MemoryBudget::set_budget(const String &p_type, const size_t p_budget) {
	MutexLock lock(*_cache_mutex.ptr());
	_budgets.insert(p_type, p_budget * 1000000);
}

bool MemoryBudget::has_budget(const String &p_type) {
	MutexLock lock(*_cache_mutex.ptr());
	if (!_budgets.has(p_type)) {
		return true;
	}

	auto budget = _budgets[p_type];
	auto current_bytes = _bytes.has(p_type) ? _bytes[p_type] : 0;
	auto estimated = _estimated.has(p_type) ? _estimated[p_type] : 0;

	return (current_bytes + estimated) <= budget;
}