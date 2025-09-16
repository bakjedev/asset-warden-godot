#pragma once
#include "godot_cpp/variant/callable.hpp"
#include "godot_cpp/variant/dictionary.hpp"
#include <godot_cpp/classes/mutex.hpp>
#include <godot_cpp/classes/semaphore.hpp>
#include <godot_cpp/classes/thread.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <queue>

namespace godot {

class AssetLoader : public Object {
	GDCLASS(AssetLoader, Object)

	static AssetLoader *singleton;

public:
	struct LoadRequest {
		uint64_t id;
		String path;
		Thread::Priority priority;
		String type;
		Callable callback;

		bool operator<(const LoadRequest &other) const {
			return (priority < other.priority);
		}
	};

	enum ThreadDistribution {
		DIST_EQUEL,
		DIST_CUSTOM
	};

	AssetLoader();
	~AssetLoader();
	static AssetLoader *get_singleton();

	void initialize(const Dictionary &p_config);
	void shutdown();

	void wake_one();
	uint64_t load(const String &p_path, const Callable &p_callback = Callable(), Thread::Priority p_priority = Thread::Priority::PRIORITY_NORMAL, const String &p_type = "");
	Array load_batch(const Array &p_paths, const Callable &p_callback = Callable(), Thread::Priority p_priority = Thread::Priority::PRIORITY_NORMAL, const String &p_type = "");

protected:
	static void _bind_methods();

private:
	Vector<Ref<Thread>> _worker_threads;
	Ref<Semaphore> _semaphore;
	std::atomic<bool> _should_exit;

	HashMap<String, std::priority_queue<LoadRequest>> _asset_type_queues;

	Ref<Mutex> _queue_mutex;

	uint64_t _next_request_id;

	void _worker_thread_func(const String &p_type);

	void create_worker_thread(const String &p_type, Thread::Priority p_priority);
};

} // namespace godot

VARIANT_ENUM_CAST(godot::AssetLoader::ThreadDistribution);
