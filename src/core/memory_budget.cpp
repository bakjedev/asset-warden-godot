#include "memory_budget.h"
#include "godot_cpp/classes/image.hpp"
#include "godot_cpp/classes/object.hpp"
#include "godot_cpp/classes/ref.hpp"
#include "godot_cpp/classes/resource_loader.hpp"
#include "godot_cpp/core/mutex_lock.hpp"
#include "godot_cpp/core/object.hpp"
#include "godot_cpp/core/object_id.hpp"
#include "size_calculator.h"

using namespace godot;

void MemoryBudget::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_mb", "type", "mb"), &MemoryBudget::set_budget);
	ClassDB::bind_method(D_METHOD("process_pending_resources"), &MemoryBudget::process_pending_resources);
}

MemoryBudget::MemoryBudget() {
	_budget_mutex.instantiate();
}

// this function will be called from separate worker threads.
void MemoryBudget::register_resource(const Ref<Resource> &p_resource) {
	if (!p_resource.is_valid()) {
		return;
	}

	auto id = ObjectID{ p_resource->get_instance_id() };
	auto type = p_resource->get_class();
	auto path = p_resource->get_path();
	auto estimated_size = SizeCalculator::estimate_resource(path);

	MutexLock lock(*_budget_mutex.ptr());

	if (_resources.has(p_resource->get_instance_id())) {
		release_reservation(type, path);
		return;
	}

	ResourceEntry entry;
	entry.id = id;
	entry.type = type;
	entry.path = path;
	entry.size_state = ResourceEntry::SizeState::ESTIMATED;
	entry.estimated_size = estimated_size;

	_resources.insert(id, entry);
}

size_t MemoryBudget::sizes(const String &p_type) const {
	if (p_type.is_empty()) {
		size_t bytes = 0;
		for (const auto &type : _sizes) {
			bytes += type.value;
		}
		return bytes;
	}

	return _sizes[p_type];
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
	struct ProcessEntry {
		uint64_t id;
		StringName type;
		size_t size;
		size_t estimated_size;
		bool valid;
	};

	Vector<ProcessEntry> to_process;
	{
		MutexLock lock(*_budget_mutex.ptr());

		for (const auto &[id, entry] : _resources) {
			if (entry.size_state == ResourceEntry::SizeState::ESTIMATED) {
				ProcessEntry process;
				process.id = id;
				process.type = entry.type;
				process.estimated_size = entry.estimated_size;

				to_process.push_back(process);
				if (to_process.size() >= p_max) {
					break;
				}
			}
		}
	}

	for (int i = 0; i < to_process.size(); ++i) {
		auto obj = ObjectDB::get_instance(to_process[i].id);
		Ref<Resource> resource = Object::cast_to<Resource>(obj);

		to_process.write[i].valid = (obj && resource.is_valid());

		if (to_process[i].valid) {
			to_process.write[i].size = SizeCalculator::calculate_resource(resource);
		}
	}

	{
		MutexLock lock(*_budget_mutex.ptr());
		for (const ProcessEntry &proc : to_process) {
			if (!_resources.has(proc.id)) {
				continue;
			}

			if (_estimated.has(proc.type) && _estimated[proc.type] >= proc.estimated_size) {
				_estimated[proc.type] -= proc.estimated_size;
			}

			if (!proc.valid) {
				_resources.erase(proc.id);
				continue;
			}

			ResourceEntry &entry = _resources[proc.id];
			entry.size = proc.size;
			entry.size_state = ResourceEntry::SizeState::KNOWN;

			if (_sizes.has(proc.type)) {
				_sizes[proc.type] += proc.size;
			} else {
				_sizes[proc.type] = proc.size;
			}
		}
	}

	_remove_counter++;
	if (_remove_counter >= _remove_interval) {
		_remove_counter = 0;

		MutexLock lock(*_budget_mutex.ptr());

		Vector<uint64_t> to_remove;
		for (auto it = _resources.begin(); it != _resources.end(); ++it) {
			Object *obj = ObjectDB::get_instance(it->key);
			if (!obj || obj->is_queued_for_deletion()) {
				to_remove.push_back(it->key);
			}
		}

		for (const auto &key : to_remove) {
			const auto &entry = _resources[key];
			if (entry.size_state == ResourceEntry::SizeState::ESTIMATED) {
				if (_estimated.has(entry.type) && _estimated[entry.type] >= entry.estimated_size) {
					_estimated[entry.type] -= entry.estimated_size;
				}
			} else if (entry.size_state == ResourceEntry::SizeState::KNOWN) {
				if (_sizes.has(entry.type) && _sizes[entry.type] >= entry.size) {
					_sizes[entry.type] -= entry.size;
				}
			}
			_resources.erase(key);
		}
	}
}

void MemoryBudget::set_budget(const String &p_type, const size_t p_budget) {
	MutexLock lock(*_budget_mutex.ptr());
	_budgets.insert(p_type, p_budget * 1000000);
}

bool MemoryBudget::reserve_budget(const String &p_type, const String &p_path) {
	MutexLock lock(*_budget_mutex.ptr());
	if (!_budgets.has(p_type)) {
		return true;
	}

	auto budget = _budgets[p_type];
	auto current_bytes = _sizes.has(p_type) ? _sizes[p_type] : 0;
	auto estimated = _estimated.has(p_type) ? _estimated[p_type] : 0;
	auto estimated_size = SizeCalculator::estimate_resource(p_path);

	if ((current_bytes + estimated + estimated_size) > budget) {
		return false;
	}

	_estimated[p_type] = estimated + estimated_size;

	return true;
}

void MemoryBudget::release_reservation(const String &p_type, const String &p_path) {
	MutexLock lock(*_budget_mutex.ptr());

	auto estimated_size = SizeCalculator::estimate_resource(p_path);

	if (_estimated.has(p_type) && _estimated[p_type] >= estimated_size) {
		_estimated[p_type] -= estimated_size;
	}
}