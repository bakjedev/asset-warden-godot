#pragma once
#include "godot_cpp/classes/ref.hpp"
#include "godot_cpp/classes/resource.hpp"
#include "godot_cpp/variant/callable.hpp"
#include "godot_cpp/variant/dictionary.hpp"
#include <cstdint>
#include <godot_cpp/classes/mutex.hpp>
#include <godot_cpp/classes/semaphore.hpp>
#include <godot_cpp/classes/thread.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <queue>
#include <unordered_map>
#include <vector>

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

	struct Batch {
		uint64_t id;
		Callable callback;
		std::vector<uint64_t> request_ids;
		int completed;
		int total;
	};

	enum ThreadDistribution {
		DIST_EQUEL,
		DIST_CUSTOM
	};

	enum RequestStatus {
		STATUS_NONE,
		STATUS_LOADING,
		STATUS_LOADED,
		STATUS_ERROR
	};

	AssetLoader();
	~AssetLoader();
	static AssetLoader *get_singleton();

	void initialize(const Dictionary &p_config);

	uint64_t load(const String &p_path, const Callable &p_callback = Callable(), Thread::Priority p_priority = Thread::PRIORITY_NORMAL, const String &p_type = "");
	uint64_t load_batch(const Array &p_paths, const Callable &p_callback = Callable(), Thread::Priority p_priority = Thread::PRIORITY_NORMAL, const String &p_type = "");

	int status(uint64_t id);
	Ref<Resource> get(uint64_t id);

	Array get_batch(uint64_t id);

protected:
	static void _bind_methods();

private:
	std::vector<Ref<Thread>> _worker_threads;
	Ref<Semaphore> _semaphore;
	std::atomic<bool> _should_exit;

	std::unordered_map<std::string, std::priority_queue<LoadRequest>> _asset_type_queues;
	std::unordered_map<uint64_t, Ref<Resource>> _completed_loads;
	std::unordered_map<uint64_t, int> _request_status;
	std::unordered_map<uint64_t, Batch> _batches;

	Ref<Mutex> _queue_mutex;
	Ref<Mutex> _cache_mutex;
	Ref<Mutex> _batch_mutex;

	uint64_t _next_request_id;
	uint64_t _next_batch_id;

	void shutdown();

	void _worker_thread_func(const String &p_type);
	void batch_item_load(Ref<Resource> p_resource, const String &p_path,
			int p_status, uint64_t id);

	void create_worker_thread(const String &p_type, Thread::Priority p_priority);
};

} // namespace godot

VARIANT_ENUM_CAST(godot::AssetLoader::ThreadDistribution);
VARIANT_ENUM_CAST(godot::AssetLoader::RequestStatus);