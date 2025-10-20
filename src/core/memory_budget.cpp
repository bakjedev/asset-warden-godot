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

void MemoryBudget::_bind_methods() {}

MemoryBudget::MemoryBudget() {
	_cache_mutex.instantiate();
}

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

void MemoryBudget::process_pending_resources(const int p_max) {
	Vector<Ref<Resource>> to_process;

	{
		MutexLock lock(*_cache_mutex.ptr());

		auto count = MIN(_pending_resources.size(), p_max);

		auto it = _pending_resources.begin();
		for (size_t i = 0; i < count && it != _pending_resources.end(); ++i, ++it) {
			to_process.push_back(*it);
		}

		for (const auto &res : to_process) {
			_pending_resources.erase(res);
		}
	}

	for (const auto &resource : to_process) {
		auto size = _get_size(resource);
		_bytes += size;
		_cache.insert(ObjectID{ resource->get_instance_id() }, size);
	}

	for (auto it = _cache.begin(); it != _cache.end(); ++it) {
		Object *obj = ObjectDB::get_instance(it->key);
		if (!obj) {
			_bytes -= it->value;
			_cache.erase(it->key);
			UtilityFunctions::print("ERASED OBJECT");
		}
	}
}