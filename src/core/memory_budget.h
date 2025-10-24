#pragma once
#include "godot_cpp/classes/mutex.hpp"
#include "godot_cpp/core/object_id.hpp"
#include "godot_cpp/templates/hash_map.hpp"
#include "godot_cpp/variant/string.hpp"
#include "godot_cpp/variant/string_name.hpp"
#include <cstddef>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/resource.hpp>

namespace godot {

class MemoryBudget : public RefCounted {
	GDCLASS(MemoryBudget, RefCounted)

private:
	struct CacheEntry {
		size_t bytes;
		StringName type;
	};

	struct PendingEntry {
		ObjectID id;
		StringName type;
		size_t estimated;
	};

	Ref<Mutex> _cache_mutex;
	HashMap<StringName, size_t> _estimated;
	Vector<PendingEntry> _pending_resources;
	HashMap<ObjectID, CacheEntry> _cache;

	HashMap<String, size_t> _budgets;
	HashMap<String, size_t> _bytes;

	size_t _get_size(const Ref<Resource> &p_resource) const;
	size_t _get_estimated_size(const String &p_path) const;

protected:
	static void _bind_methods();

public:
	MemoryBudget();

	void register_resource(const Ref<Resource> &p_resource);

	size_t bytes(const String &p_type = "") const;
	size_t estimated(const String &p_type = "") const;

	void process_pending_resources(int p_max = 5);

	void set_budget(const String &p_type, size_t p_budget);

	bool reserve_budget(const String &p_type, const String &p_path);
	void release_reservation(const String &p_type, const String &p_path);
};

} // namespace godot
