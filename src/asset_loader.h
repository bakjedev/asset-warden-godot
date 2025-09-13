#pragma once
#include <godot_cpp/classes/semaphore.hpp>
#include <godot_cpp/classes/thread.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/templates/vector.hpp>

namespace godot {

class AssetLoader : public Object {
	GDCLASS(AssetLoader, Object)

	static AssetLoader *singleton;

public:
	AssetLoader();
	~AssetLoader();
	static AssetLoader *get_singleton();

	void initialize(int p_thread_count);
	void shutdown();

	void wake_one();

protected:
	static void _bind_methods();

private:
	Vector<Ref<Thread>> _worker_threads;
	Ref<Semaphore> _semaphore;
	std::atomic<bool> _should_exit;

	void _worker_thread_func();
};

} // namespace godot
