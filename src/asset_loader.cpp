#include "asset_loader.h"
#include "godot_cpp/classes/global_constants.hpp"
#include "godot_cpp/classes/os.hpp"
#include "godot_cpp/classes/resource.hpp"
#include "godot_cpp/classes/resource_loader.hpp"
#include "godot_cpp/classes/thread.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/core/mutex_lock.hpp"
#include "godot_cpp/variant/callable.hpp"
#include "godot_cpp/variant/dictionary.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include <cstdint>
#include <queue>

namespace godot {

AssetLoader *AssetLoader::singleton = nullptr;

void AssetLoader::initialize(const Dictionary &p_config) {
	shutdown();

	_semaphore.instantiate();
	_queue_mutex.instantiate();

	auto core_count = OS::get_singleton()->get_processor_count();
	auto available_cores = Math::max(1, core_count - 2);

	if (p_config.has("distribution")) {
		auto distribution = static_cast<ThreadDistribution>(static_cast<int>(p_config["distribution"]));

		switch (distribution) {
			case DIST_EQUEL: {
				for (int i = 0; i < available_cores; ++i) {
					create_worker_thread("", Thread::PRIORITY_NORMAL);
				}
				break;
			}
			case DIST_CUSTOM: {
				if (p_config.has("pools")) {
					Array pools = p_config["pools"];
					for (int i = 0; i < pools.size(); ++i) {
						Dictionary pool = pools[i];
						for (int j = 0; j < static_cast<int>(pool["count"]); ++j) {
							create_worker_thread(pool["type"], static_cast<Thread::Priority>(static_cast<int>(pool["priority"])));
						}
					}
				}
				break;
			}
		}
	}
}

void AssetLoader::shutdown() {
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
		auto &load_queue = asset_queue.value;
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

void AssetLoader::wake_one() {
	_semaphore->post(1);
}

uint64_t AssetLoader::load(const String &p_path, const Callable &p_callback, Thread::Priority p_priority, const String &p_type) {
	LoadRequest request{ _next_request_id++, p_path, p_priority, p_type, p_callback };

	{
		MutexLock lock(*_queue_mutex.ptr());
		if (p_type == "texture") {
			_asset_type_queues["texture"].push(request);
		} else if (p_type == "mesh") {
			_asset_type_queues["mesh"].push(request);
		} else {
			return 0;
		}
	}

	_semaphore->post();

	return request.id;
}

Array AssetLoader::load_batch(const Array &p_paths, const Callable &p_callback, Thread::Priority p_priority, const String &p_type) {
	Array request_ids;
	request_ids.resize(p_paths.size());

	{
		MutexLock lock(*_queue_mutex.ptr());

		for (int i = 0; i < p_paths.size(); ++i) {
			LoadRequest request{ _next_request_id++, p_paths[i], p_priority, p_type, p_callback };

			if (p_type == "texture") {
				_asset_type_queues["texture"].push(request);
			} else if (p_type == "mesh") {
				_asset_type_queues["mesh"].push(request);
			}

			request_ids[i] = request.id;
		}
	}

	_semaphore->post(p_paths.size());

	return request_ids;
}

void AssetLoader::_bind_methods() {
	ClassDB::bind_static_method("AssetLoader", D_METHOD("get_singleton"),
			&AssetLoader::get_singleton);
	ClassDB::bind_method(D_METHOD("_worker_thread_func"), &AssetLoader::_worker_thread_func);
	ClassDB::bind_method(D_METHOD("initialize", "config"), &AssetLoader::initialize);
	ClassDB::bind_method(D_METHOD("shutdown"), &AssetLoader::shutdown);
	ClassDB::bind_method(D_METHOD("wake_one"), &AssetLoader::wake_one);
	ClassDB::bind_method(D_METHOD("load", "path", "callback", "priority", "type"), &AssetLoader::load);
	ClassDB::bind_method(D_METHOD("load_batch", "paths", "callback", "priority", "type"), &AssetLoader::load_batch);
	BIND_ENUM_CONSTANT(DIST_EQUEL);
	BIND_ENUM_CONSTANT(DIST_CUSTOM);
}

AssetLoader::AssetLoader() :
		_should_exit(false), _next_request_id(1) {
	singleton = this;
}

AssetLoader::~AssetLoader() {
	singleton = nullptr;
	shutdown();
}

AssetLoader *AssetLoader::get_singleton() {
	return singleton;
}

void AssetLoader::_worker_thread_func(const String &p_type) {
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
					if (!load_queue.value.empty()) {
						request = load_queue.value.top();
						load_queue.value.pop();
						has_request = true;
						break;
					}
				}
			}
		}

		if (has_request) {
			auto res = ResourceLoader::get_singleton()->load(request.path, request.type);
			//UtilityFunctions::print("Loaded ", request.path);

			if (request.callback.is_valid()) {
				request.callback.call_deferred(res, request.path, res.is_valid() ? OK : ERR_FILE_NOT_FOUND);
			}
		}
	}

	UtilityFunctions::print("Ending thread");
}

void AssetLoader::create_worker_thread(const String &p_type, Thread::Priority p_priority) {
	Ref<Thread> worker_thread;
	worker_thread.instantiate();
	worker_thread->start(Callable(this, "_worker_thread_func").bind(p_type), p_priority);
	_worker_threads.push_back(worker_thread);
}
} // namespace godot