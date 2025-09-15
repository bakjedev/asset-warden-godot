#pragma once
#include "godot_cpp/variant/callable.hpp"
#include <godot_cpp/classes/mutex.hpp>
#include <godot_cpp/classes/semaphore.hpp>
#include <godot_cpp/classes/thread.hpp>
#include <godot_cpp/core/object.hpp>
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
		String type_hint;
		Callable callback;

		bool operator<(const LoadRequest &other) const {
			return (priority < other.priority);
		}
	};

	AssetLoader();
	~AssetLoader();
	static AssetLoader *get_singleton();

	void initialize(int p_thread_count);
	void shutdown();

	void wake_one();
	uint64_t load(const String &p_path, const Callable &p_callback = Callable(), Thread::Priority p_priority = Thread::Priority::PRIORITY_NORMAL, const String &p_type_hint = "");

protected:
	static void _bind_methods();

private:
	Vector<Ref<Thread>> _worker_threads;
	Ref<Semaphore> _semaphore;
	std::atomic<bool> _should_exit;

	std::priority_queue<LoadRequest> _load_queue;
	Ref<Mutex> _queue_mutex;

	uint64_t _next_request_id;

	void _worker_thread_func(int p_index);
};

} // namespace godot
