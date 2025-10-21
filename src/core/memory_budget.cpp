#include "memory_budget.h"
#include "godot_cpp/classes/array_mesh.hpp"
#include "godot_cpp/classes/file_access.hpp"
#include "godot_cpp/classes/image.hpp"
#include "godot_cpp/classes/image_texture.hpp"
#include "godot_cpp/classes/ref.hpp"
#include "godot_cpp/classes/resource_loader.hpp"
#include "godot_cpp/core/defs.hpp"
#include "godot_cpp/core/mutex_lock.hpp"
#include "godot_cpp/core/object.hpp"
#include "godot_cpp/variant/utility_functions.hpp"

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

void MemoryBudget::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_mb", "type", "mb"), &MemoryBudget::set_budget);
}

MemoryBudget::MemoryBudget() {
	_cache_mutex.instantiate();
}

// this function will be called from separate worker threads.
void MemoryBudget::register_resource(const Ref<Resource> &p_resource) {
	if (!p_resource.is_valid()) {
		return;
	}

	MutexLock lock(*_cache_mutex.ptr());
	if (_cache.has(ObjectID{ p_resource->get_instance_id() })) {
		return;
	}

	_pending_resources.insert(p_resource);
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

// this function will be run on the main thread
void MemoryBudget::process_pending_resources(const int p_max) {
	Vector<Ref<Resource>> to_process;

	{
		MutexLock lock(*_cache_mutex.ptr());

		auto count = MIN(_pending_resources.size(), p_max);

		auto it = _pending_resources.begin();
		for (size_t i = 0; i < count; ++i, ++it) {
			to_process.push_back(*it);
		}

		for (const auto &res : to_process) {
			_pending_resources.erase(res);
		}
	}

	// out of lock cuz _get_size is heavy
	HashMap<ObjectID, std::pair<size_t, String>> sizes_to_add;
	for (const auto &resource : to_process) {
		sizes_to_add.insert(ObjectID{ resource->get_instance_id() }, { _get_size(resource), resource->get_class() });
	}

	{
		MutexLock lock(*_cache_mutex.ptr());
		for (const auto &[id, res] : sizes_to_add) {
			_bytes[res.second] += res.first;
			_cache.insert(id, { res.first, res.second });
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
			UtilityFunctions::print("ERASED OBJECT");
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

	return current_bytes <= budget;
}