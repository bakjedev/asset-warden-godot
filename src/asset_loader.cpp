#include "asset_loader.h"

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/print_string.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <sstream>

#include "godot_cpp/classes/engine.hpp"
#include "godot_cpp/classes/global_constants.hpp"
#include "godot_cpp/variant/callable.hpp"


namespace godot {

AssetLoader* AssetLoader::singleton = nullptr;

void AssetLoader::_bind_methods() {
    ClassDB::bind_static_method("AssetLoader", D_METHOD("get_singleton"),
                                &AssetLoader::get_singleton);
    ClassDB::bind_method(D_METHOD("load", "path", "callback", "priority", "type_hint"),
                         &AssetLoader::load, DEFVAL(Callable()), DEFVAL(PRIORITY_NORMAL),
                         DEFVAL(""));
    ClassDB::bind_method(D_METHOD("initialize_worker_threads", "count"),
                         &AssetLoader::initialize_worker_threads);
    ClassDB::bind_method(D_METHOD("is_loading", "request_id"), &AssetLoader::is_loading);
    ClassDB::bind_method(D_METHOD("get_cpu_count"), &AssetLoader::get_cpu_count);


    BIND_ENUM_CONSTANT(PRIORITY_LOW);
    BIND_ENUM_CONSTANT(PRIORITY_NORMAL);
    BIND_ENUM_CONSTANT(PRIORITY_HIGH);
}

AssetLoader::AssetLoader() : should_exit(false), next_request_id(1) {
    singleton = this;

    initialize_worker_threads(2);
}

AssetLoader::~AssetLoader() {
    should_exit = true;
    cv.notify_all();

    for (auto& thread : worker_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    singleton = nullptr;
}

AssetLoader* AssetLoader::get_singleton() {
    return singleton;
}

uint64_t AssetLoader::load(const String& p_path, const Callable& p_callback,
                           LoadPriority p_priority, const String& p_type_hint) {
    LoadRequest request;
    request.path = p_path;
    request.type_hint = p_type_hint;
    request.priority = p_priority;
    request.callback = p_callback;
    request.request_id = next_request_id++;

    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        load_queue.push(request);
        active_requests[request.request_id] = request;
    }

    cv.notify_one();

    return request.request_id;
}

void AssetLoader::initialize_worker_threads(int p_count) {
    should_exit = true;
    cv.notify_all();

    for (auto& thread : worker_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    worker_threads.clear();
    should_exit = false;

    for (int i = 0; i < p_count; ++i) {
        worker_threads.emplace_back(&AssetLoader::worker_thread_func, this);
    }
}

bool AssetLoader::is_loading(uint64_t p_request_id) const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(queue_mutex));
    return active_requests.has(p_request_id);
}

int AssetLoader::get_cpu_count() const {
    return std::thread::hardware_concurrency();
}

void AssetLoader::worker_thread_func() {
    std::thread::id thread_id = std::this_thread::get_id();
    std::stringstream ss;
    ss << thread_id;
    String thread_id_str = String(ss.str().c_str());

    while (!should_exit) {
        LoadRequest request;
        bool has_request = false;

        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            cv.wait(lock, [this] { return !load_queue.empty() || should_exit; });

            if (!load_queue.empty()) {
                request = load_queue.top();
                load_queue.pop();
                has_request = true;
            }
        }

        if (has_request && !should_exit) {
            print_line("Thread " + thread_id_str + " processing: " + request.path);


            ResourceLoader* loader = ResourceLoader::get_singleton();
            Ref<Resource> resource = loader->load(request.path, request.type_hint);

            // Remove from active requests
            {
                std::lock_guard<std::mutex> lock(queue_mutex);
                active_requests.erase(request.request_id);
            }

            // Call the callback directly on main thread using call_deferred
            if (request.callback.is_valid()) {
                // This will execute on the main thread!
                request.callback.call_deferred(resource, request.path,
                                               resource.is_valid() ? OK : ERR_FILE_NOT_FOUND);
            }
        }
    }
}
} // namespace godot