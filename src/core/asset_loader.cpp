#include "asset_loader.h"
#include "debugger/debug_sender.h"
#include "godot_cpp/classes/engine.hpp"
#include "godot_cpp/classes/global_constants.hpp"
#include "godot_cpp/classes/main_loop.hpp"
#include "godot_cpp/classes/os.hpp"
#include "godot_cpp/classes/resource.hpp"
#include "godot_cpp/classes/resource_loader.hpp"
#include "godot_cpp/classes/scene_tree.hpp"
#include "godot_cpp/classes/thread.hpp"
#include "godot_cpp/classes/timer.hpp"
#include "godot_cpp/classes/window.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/core/error_macros.hpp"
#include "godot_cpp/core/memory.hpp"
#include "godot_cpp/core/mutex_lock.hpp"
#include "godot_cpp/variant/callable.hpp"
#include "godot_cpp/variant/dictionary.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include <cstdint>
#include <queue>

using namespace godot;

AssetLoader *AssetLoader::singleton = nullptr;

void AssetLoader::initialize(const Dictionary &p_config) {
	_shutdown();

	_semaphore.instantiate();
	_queue_mutex.instantiate();
	_cache_mutex.instantiate();
	_batch_mutex.instantiate();
	_debug_sender = DebugSender::create("bakjetest");
	_memory_budget.instantiate();

	_setup_process_loop();

	auto core_count = OS::get_singleton()->get_processor_count();
	auto available_cores = Math::max(1, core_count - 2);

	if (p_config.has("distribution")) {
		auto distribution = static_cast<ThreadDistribution>(static_cast<int>(p_config["distribution"]));

		switch (distribution) {
			case DIST_EQUAL: {
				for (int i = 0; i < available_cores; ++i) {
					_create_worker_thread("", Thread::PRIORITY_NORMAL);
				}
				break;
			}
			case DIST_CUSTOM: {
				if (p_config.has("pools")) {
					Array pools = p_config["pools"];
					for (int i = 0; i < pools.size(); ++i) {
						Dictionary pool = pools[i];
						for (int j = 0; j < static_cast<int>(pool["count"]); ++j) {
							_create_worker_thread(pool["type"], static_cast<Thread::Priority>(static_cast<int>(pool["priority"])));
						}
					}
				}
				break;
			}
		}
	}
}

void AssetLoader::_shutdown() {
	_should_exit = true;

	if (_semaphore.is_valid()) {
		_semaphore->post(_worker_threads.size());
	}

	for (auto wt : _worker_threads) {
		if (wt.is_valid()) {
			wt->wait_to_finish();
		}
	}

	_worker_threads.clear();

	for (auto &asset_queue : _asset_type_queues) {
		auto &load_queue = asset_queue.second;
		if (!load_queue.empty() && _queue_mutex.is_valid()) {
			MutexLock lock(*_queue_mutex.ptr());
			while (!load_queue.empty()) {
				load_queue.pop();
			}
		}
	}

	_should_exit = false;
	_next_request_id = 1;
}

uint64_t AssetLoader::load(const String &p_path, const StringName &p_type, Thread::Priority p_priority, const Callable &p_callback) {
	LoadRequest request{ _next_request_id++, p_path, p_priority, p_type, p_callback };

	{
		MutexLock lock(*_queue_mutex.ptr());
		_asset_type_queues[p_type].push(request);
	}

	_semaphore->post();

	return request.id;
}

uint64_t AssetLoader::load_batch(const Array &p_paths, const StringName &p_type, Thread::Priority p_priority, const Callable &p_callback, const Callable &p_batch_callback) {
	uint64_t batch_id = _next_batch_id++;
	Batch batch;
	batch.id = batch_id;
	batch.callback = p_batch_callback;
	batch.total = p_paths.size();
	batch.completed = 0;
	batch.errors = false;

	for (int i = 0; i < p_paths.size(); ++i) {
		String path = p_paths[i];

		auto batch_callback = callable_mp(this, &AssetLoader::_batch_item_load).bind(batch_id, p_callback);

		auto request_id = load(path, p_type, p_priority, batch_callback);
		batch.request_ids.push_back(request_id);
	}

	{
		MutexLock lock(*_batch_mutex.ptr());
		_batches[batch_id] = batch;
	}

	return batch_id;
}

int AssetLoader::status(uint64_t p_id) {
	MutexLock lock(*_cache_mutex.ptr());
	auto it = _request_status.find(p_id);
	if (it != _request_status.end()) {
		return _request_status[p_id];
	}
	return STATUS_NONE;
}

Ref<Resource> AssetLoader::get(uint64_t p_id) {
	MutexLock lock(*_cache_mutex.ptr());
	auto it = _completed_loads.find(p_id);
	if (it == _completed_loads.end()) {
		return Ref<Resource>();
	}

	auto resource = _completed_loads[p_id];
	if (!resource.is_valid()) {
		return Ref<Resource>();
	}

	_completed_loads.erase(p_id);
	_request_status.erase(p_id);

	return resource;
}

Array AssetLoader::get_batch(uint64_t p_id) {
	Array result;

	MutexLock batch_lock(*_batch_mutex.ptr());

	auto batch_it = _batches.find(p_id);
	if (batch_it == _batches.end()) {
		return result;
	}

	const auto &batch = batch_it->second;
	if (batch.completed < batch.total) {
		return result;
	}

	MutexLock cache_lock(*_cache_mutex.ptr());
	for (auto request_id : batch.request_ids) {
		auto load_it = _completed_loads.find(request_id);
		if (load_it != _completed_loads.end()) {
			result.append(load_it->second);
		}
	}

	for (auto request_id : batch.request_ids) {
		_completed_loads.erase(request_id);
		_request_status.erase(request_id);
	}

	_batches.erase(p_id);

	return result;
}

int AssetLoader::status_batch(uint64_t p_id) {
	MutexLock lock(*_batch_mutex.ptr());

	auto batch_it = _batches.find(p_id);
	if (batch_it == _batches.end()) {
		return STATUS_NONE;
	}

	const auto &batch = batch_it->second;
	if (batch.completed == 0) {
		return STATUS_LOADING;
	} else if (batch.completed >= batch.total) {
		return batch.errors ? STATUS_ERROR : STATUS_LOADED;
	} else {
		return STATUS_LOADING;
	}
}

float AssetLoader::progress_batch(uint64_t p_id) {
	MutexLock lock(*_batch_mutex.ptr());
	auto it = _batches.find(p_id);
	if (it == _batches.end()) {
		return 0.0f;
	}

	const auto &batch = it->second;
	if (batch.total == 0) {
		return 0.0f;
	}

	return static_cast<float>(batch.completed) / static_cast<float>(batch.total);
}

void AssetLoader::cancel(uint64_t p_id) {
	MutexLock cache_lock(*_cache_mutex.ptr());
	_completed_loads.erase(p_id);
	_request_status[p_id] = STATUS_CANCELLED;
}

void AssetLoader::cancel_batch(uint64_t p_id) {
	MutexLock batch_lock(*_batch_mutex.ptr());
	auto batch_it = _batches.find(p_id);
	if (batch_it == _batches.end()) {
		return;
	}

	const auto &batch = batch_it->second;
	{
		MutexLock cache_lock(*_cache_mutex.ptr());
		for (auto request_id : batch.request_ids) {
			_completed_loads.erase(request_id);
			_request_status[request_id] = STATUS_CANCELLED;
		}
	}

	_batches.erase(p_id);
}

void AssetLoader::_bind_methods() {
	ClassDB::bind_static_method("AssetLoader", D_METHOD("get_singleton"),
			&AssetLoader::get_singleton);
	ClassDB::bind_method(D_METHOD("_worker_thread_func"), &AssetLoader::_worker_thread_func);
	ClassDB::bind_method(D_METHOD("initialize", "config"), &AssetLoader::initialize);
	ClassDB::bind_method(D_METHOD("load", "path", "callback", "priority", "type"), &AssetLoader::load);
	ClassDB::bind_method(D_METHOD("load_batch", "paths", "callback", "batch_callback", "priority", "type"), &AssetLoader::load_batch);
	ClassDB::bind_method(D_METHOD("status", "id"), &AssetLoader::status);
	ClassDB::bind_method(D_METHOD("get", "id"), &AssetLoader::get);
	ClassDB::bind_method(D_METHOD("get_batch", "id"), &AssetLoader::get_batch);
	ClassDB::bind_method(D_METHOD("status_batch", "id"), &AssetLoader::status_batch);
	ClassDB::bind_method(D_METHOD("progress_batch", "id"), &AssetLoader::progress_batch);
	ClassDB::bind_method(D_METHOD("cancel", "id"), &AssetLoader::cancel);
	ClassDB::bind_method(D_METHOD("cancel_batch", "id"), &AssetLoader::cancel_batch);
	ClassDB::bind_method(D_METHOD("_batch_item_load"), &AssetLoader::_batch_item_load);

	BIND_ENUM_CONSTANT(DIST_EQUAL);
	BIND_ENUM_CONSTANT(DIST_CUSTOM);

	BIND_ENUM_CONSTANT(STATUS_NONE);
	BIND_ENUM_CONSTANT(STATUS_LOADING);
	BIND_ENUM_CONSTANT(STATUS_LOADED);
	BIND_ENUM_CONSTANT(STATUS_ERROR);
	BIND_ENUM_CONSTANT(STATUS_CANCELLED);
}

AssetLoader::AssetLoader() :
		_should_exit(false), _next_request_id(1), _next_batch_id(1) {
	ERR_FAIL_COND(singleton != nullptr);
	singleton = this;
}

AssetLoader::~AssetLoader() {
	ERR_FAIL_COND(singleton != this);
	singleton = nullptr;
	_shutdown();
}

AssetLoader *AssetLoader::get_singleton() {
	return singleton;
}

void AssetLoader::_worker_thread_func(const StringName &p_type) {
	UtilityFunctions::print("Starting thread");

	while (!_should_exit) {
		_semaphore->wait();

		if (_should_exit) {
			break;
		}

		LoadRequest request;
		bool has_request = false;

		{
			MutexLock lock(*_queue_mutex.ptr());
			if (!_asset_type_queues[p_type].empty()) {
				request = _asset_type_queues[p_type].top();
				_asset_type_queues[p_type].pop();
				has_request = true;
			} else {
				for (auto &load_queue : _asset_type_queues) {
					if (!load_queue.second.empty()) {
						request = load_queue.second.top();
						load_queue.second.pop();
						has_request = true;
						break;
					}
				}
			}
		}

		if (has_request) {
			{
				MutexLock lock(*_cache_mutex.ptr());
				if (_request_status[request.id] == STATUS_CANCELLED) {
					_request_status.erase(request.id);
					continue;
				}
				_request_status[request.id] = STATUS_LOADING;
			}
			auto res = ResourceLoader::get_singleton()->load(request.path, request.type);

			{
				MutexLock lock(*_cache_mutex.ptr());
				if (res.is_valid()) {
					_completed_loads[request.id] = res;
					_request_status[request.id] = STATUS_LOADED;
					_memory_budget->register_resource(res);
				} else {
					_request_status[request.id] = STATUS_ERROR;
				}
			}

			if (request.callback.is_valid()) {
				request.callback.call_deferred(res, request.path, res.is_valid() ? OK : ERR_FILE_NOT_FOUND);
			}
		}
	}

	UtilityFunctions::print("Ending thread");
}

void AssetLoader::_batch_item_load(Ref<Resource> p_resource, const String &p_path,
		int p_status, uint64_t p_id, const Callable &p_callback) {
	bool batch_complete = false;
	Batch batch_copy;

	{
		MutexLock lock(*_batch_mutex.ptr());
		auto it = _batches.find(p_id);
		if (it == _batches.end()) {
			return;
		}

		Batch &batch = _batches[p_id];
		batch.completed++;

		if (p_status != OK && !batch.errors) {
			batch.errors = true;
		}

		if (batch.completed >= batch.total) {
			batch_copy = batch;
			batch_complete = true;
		}
	}

	if (p_callback.is_valid()) {
		p_callback.call_deferred(p_resource);
	}

	if (batch_complete && batch_copy.callback.is_valid()) {
		auto resources = get_batch(p_id);
		batch_copy.callback.call_deferred(resources);
	}
}

void AssetLoader::_create_worker_thread(const StringName &p_type, Thread::Priority p_priority) {
	Ref<Thread> worker_thread;
	worker_thread.instantiate();
	worker_thread->start(Callable(this, "_worker_thread_func").bind(p_type), p_priority);
	_worker_threads.push_back(worker_thread);
}

void AssetLoader::_setup_process_loop() {
	auto *main_loop = Engine::get_singleton()->get_main_loop();
	auto *scene_tree = Object::cast_to<SceneTree>(main_loop);

	_node = memnew(AssetLoaderNode);
	_node->asset_loader = this;

	if (_node) {
		scene_tree->get_root()->call_deferred("add_child", _node);
	}
}

void AssetLoaderNode::_process(double delta) {
	asset_loader->_memory_budget->process_pending_resources(2);
}

void AssetLoaderNode::_physics_process(double delta) {
	int request_count = 0;
	for (const auto &[asset_type, queue] : asset_loader->_asset_type_queues) {
		request_count += queue.size();
	}

	asset_loader->_debug_sender->send("request_count", request_count);
	asset_loader->_debug_sender->send("bytes", asset_loader->_memory_budget->bytes());
}