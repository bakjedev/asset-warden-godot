#include "asset_loader.h"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/variant/callable.hpp"
#include "godot_cpp/variant/utility_functions.hpp"

namespace godot {

AssetLoader *AssetLoader::singleton = nullptr;

void AssetLoader::initialize(int p_thread_count) {
	shutdown();
	for (int i = 0; i < p_thread_count; ++i) {
		Ref<Thread> worker_thread;
		worker_thread.instantiate();

		worker_thread->start(Callable(this, "_worker_thread_func"));

		_worker_threads.push_back(worker_thread);
	}

	_semaphore.instantiate();
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
	_should_exit = false;
}

void AssetLoader::wake_one() {
	_semaphore->post();
}

void AssetLoader::_bind_methods() {
	ClassDB::bind_static_method("AssetLoader", D_METHOD("get_singleton"),
			&AssetLoader::get_singleton);
	ClassDB::bind_method(D_METHOD("_worker_thread_func"), &AssetLoader::_worker_thread_func);
	ClassDB::bind_method(D_METHOD("initialize"), &AssetLoader::initialize);
	ClassDB::bind_method(D_METHOD("shutdown"), &AssetLoader::shutdown);
	ClassDB::bind_method(D_METHOD("wake_one"), &AssetLoader::wake_one);
}

AssetLoader::AssetLoader() :
		_should_exit(false) {
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

void AssetLoader::_worker_thread_func() {
	UtilityFunctions::print("Starting thread");

	while (!_should_exit) {
		_semaphore->wait();

		if (_should_exit) {
			break;
		}

		UtilityFunctions::print("s");
	}

	UtilityFunctions::print("Ending thread");
}
} // namespace godot