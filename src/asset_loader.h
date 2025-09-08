#pragma once
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/object.hpp>

#include <condition_variable>
#include <queue>
#include <thread>

#include "godot_cpp/core/binder_common.hpp"
#include "godot_cpp/templates/hash_map.hpp"
#include "godot_cpp/variant/callable.hpp"


namespace godot {

class AssetLoader : public Object {
    GDCLASS(AssetLoader, Object)

    static AssetLoader* singleton;

public:
    enum LoadPriority {
        PRIORITY_LOW = 0,
        PRIORITY_NORMAL = 1,
        PRIORITY_HIGH = 2,
    };

    struct LoadRequest {
        String path;
        String type_hint;
        LoadPriority priority;
        Callable callback;
        uint64_t request_id;

        bool operator<(const LoadRequest& other) const {
            return priority < other.priority;
        }
    };

protected:
    static void _bind_methods();

public:
    AssetLoader();
    ~AssetLoader();
    static AssetLoader* get_singleton();

    uint64_t load(const String& p_path, const Callable& p_callback = Callable(),
                  LoadPriority p_priority = PRIORITY_NORMAL, const String& p_type_hint = "");

    void initialize_worker_threads(int p_count);
    bool is_loading(uint64_t p_request_id) const;
    int get_cpu_count() const;

private:
    std::vector<std::thread> worker_threads;
    std::atomic<bool> should_exit;
    std::condition_variable cv;
    std::priority_queue<LoadRequest> load_queue;
    std::mutex queue_mutex;

    HashMap<uint64_t, LoadRequest> active_requests;
    uint64_t next_request_id;


    void worker_thread_func();
    void process_completed_loads();
};

} // namespace godot
VARIANT_ENUM_CAST(godot::AssetLoader::LoadPriority);
