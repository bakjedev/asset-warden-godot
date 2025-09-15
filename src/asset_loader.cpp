#include "asset_loader.h"
#include "godot_cpp/classes/global_constants.hpp"
#include "godot_cpp/classes/resource.hpp"
#include "godot_cpp/classes/resource_loader.hpp"
#include "godot_cpp/classes/thread.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/core/mutex_lock.hpp"
#include "godot_cpp/variant/callable.hpp"
#include "godot_cpp/variant/utility_functions.hpp"

namespace godot {

AssetLoader *AssetLoader::singleton = nullptr;

void AssetLoader::initialize(int p_thread_count) {
	shutdown();

	_semaphore.instantiate();
	_queue_mutex.instantiate();

	for (int i = 0; i < p_thread_count; ++i) {
		Ref<Thread> worker_thread;
		worker_thread.instantiate();

		worker_thread->start(Callable(this, "_worker_thread_func").bind(i));

		_worker_threads.push_back(worker_thread);
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

	if (!_load_queue.empty() && _queue_mutex.is_valid()) {
		MutexLock lock(*_queue_mutex.ptr());
		while (!_load_queue.empty()) {
			_load_queue.pop();
			UtilityFunctions::print("POPPING LOAD QUEUE");
		}
	}

	_should_exit = false;
	_next_request_id = 1;
}

void AssetLoader::wake_one() {
	_semaphore->post();
}

uint64_t AssetLoader::load(const String &p_path, const Callable &p_callback, Thread::Priority p_priority, const String &p_type_hint) {
	LoadRequest request{ _next_request_id++, p_path, p_priority, p_type_hint, p_callback };

	{
		MutexLock lock(*_queue_mutex.ptr());
		_load_queue.push(request);
	}

	_semaphore->post();

	return request.id;
}

void AssetLoader::_bind_methods() {
	ClassDB::bind_static_method("AssetLoader", D_METHOD("get_singleton"),
			&AssetLoader::get_singleton);
	ClassDB::bind_method(D_METHOD("_worker_thread_func"), &AssetLoader::_worker_thread_func);
	ClassDB::bind_method(D_METHOD("initialize"), &AssetLoader::initialize);
	ClassDB::bind_method(D_METHOD("shutdown"), &AssetLoader::shutdown);
	ClassDB::bind_method(D_METHOD("wake_one"), &AssetLoader::wake_one);
	ClassDB::bind_method(D_METHOD("load"), &AssetLoader::load);
}

AssetLoader::AssetLoader() :
		_should_exit(false), _next_request_id(1) {
	singleton = this;

	initialize(2);
}

AssetLoader::~AssetLoader() {
	singleton = nullptr;
	shutdown();
}

AssetLoader *AssetLoader::get_singleton() {
	return singleton;
}

void AssetLoader::_worker_thread_func(int p_index) {
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

			if (!_should_exit && !_load_queue.empty()) {
				request = _load_queue.top();
				_load_queue.pop();
				has_request = true;
			}
		}

		if (has_request) {
			auto res = ResourceLoader::get_singleton()->load(request.path, request.type_hint);
			//UtilityFunctions::print("Loaded ", request.path);

			if (request.callback.is_valid()) {
				request.callback.call_deferred(res, request.path, res.is_valid() ? OK : ERR_FILE_NOT_FOUND);
			}
		}
	}

	UtilityFunctions::print("Ending thread");
}
} // namespace godot