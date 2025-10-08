#pragma once
#include "debugger/debug_sender.h"
#include "godot_cpp/classes/ref.hpp"
#include "godot_cpp/classes/resource.hpp"
#include "godot_cpp/variant/callable.hpp"
#include "godot_cpp/variant/dictionary.hpp"
#include "memory_budget.h"
#include <cstdint>
#include <godot_cpp/classes/mutex.hpp>
#include <godot_cpp/classes/semaphore.hpp>
#include <godot_cpp/classes/thread.hpp>
#include <godot_cpp/classes/timer.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <queue>
#include <unordered_map>
#include <vector>

namespace godot {

class AssetLoaderNode;

class AssetLoader : public Object {
	GDCLASS(AssetLoader, Object)

	static AssetLoader *singleton;

public:
	struct LoadRequest {
		uint64_t id;
		String path;
		Thread::Priority priority;
		StringName type;
		Callable callback;

		bool operator<(const LoadRequest &other) const {
			return (priority > other.priority);
		}
	};

	struct Batch {
		uint64_t id;
		Callable callback;
		std::vector<uint64_t> request_ids;
		int completed;
		int total;
		bool errors;
	};

	enum ThreadDistribution {
		DIST_EQUAL,
		DIST_CUSTOM
	};

	enum RequestStatus {
		STATUS_NONE,
		STATUS_LOADING,
		STATUS_LOADED,
		STATUS_ERROR,
		STATUS_CANCELLED
	};

	AssetLoader();
	~AssetLoader();
	static AssetLoader *get_singleton();

	void initialize(const Dictionary &p_config);

	uint64_t load(const String &p_path, const StringName &p_type, Thread::Priority p_priority = Thread::PRIORITY_NORMAL, const Callable &p_callback = Callable());
	uint64_t load_batch(const Array &p_paths, const StringName &p_type, Thread::Priority p_priority = Thread::PRIORITY_NORMAL, const Callable &p_callback = Callable(), const Callable &p_batch_callback = Callable());

	int status(uint64_t p_id);
	Ref<Resource> get(uint64_t p_id);

	Array get_batch(uint64_t p_id);
	int status_batch(uint64_t p_id);
	float progress_batch(uint64_t p_id);

	void cancel(uint64_t p_id);
	void cancel_batch(uint64_t p_id);

protected:
	static void _bind_methods();

private:
	friend class AssetLoaderNode;

	std::vector<Ref<Thread>> _worker_threads;
	Ref<Semaphore> _semaphore;
	std::atomic<bool> _should_exit;

	std::unordered_map<StringName, std::priority_queue<LoadRequest>> _asset_type_queues;
	std::unordered_map<uint64_t, Ref<Resource>> _completed_loads;
	std::unordered_map<uint64_t, int> _request_status;
	std::unordered_map<uint64_t, Batch> _batches;

	Ref<Mutex> _queue_mutex;
	Ref<Mutex> _cache_mutex;
	Ref<Mutex> _batch_mutex;

	uint64_t _next_request_id;
	uint64_t _next_batch_id;

	Ref<DebugSender> _debug_sender;

	Ref<MemoryBudget> _memory_budget;
	AssetLoaderNode *_node = nullptr;

	void _shutdown();

	void _worker_thread_func(const StringName &p_type);
	void _batch_item_load(Ref<Resource> p_resource, const String &p_path,
			int p_status, uint64_t p_id, const Callable &p_callback);

	void _create_worker_thread(const StringName &p_type, Thread::Priority p_priority);

	void _setup_process_loop();
};

class AssetLoaderNode : public Node {
	GDCLASS(AssetLoaderNode, Node);

private:
protected:
	static void _bind_methods() {}

public:
	AssetLoader *asset_loader = nullptr;

	virtual void _process(double delta) override;
	virtual void _physics_process(double delta) override;
};

} // namespace godot

VARIANT_ENUM_CAST(godot::AssetLoader::ThreadDistribution);
VARIANT_ENUM_CAST(godot::AssetLoader::RequestStatus);