#pragma once
#include "godot_cpp/classes/mutex.hpp"
#include "godot_cpp/core/object_id.hpp"
#include "godot_cpp/templates/hash_map.hpp"
#include "godot_cpp/templates/hash_set.hpp"
#include "godot_cpp/variant/string.hpp"
#include <cstddef>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/resource.hpp>

namespace godot {

class MemoryBudget : public RefCounted {
	GDCLASS(MemoryBudget, RefCounted)

private:
	std::atomic<size_t> _bytes{ 0 };
	Ref<Mutex> _cache_mutex;
	HashSet<Ref<Resource>> _pending_resources;
	HashMap<ObjectID, size_t> _cache;

	HashMap<String, size_t> _budgets;

	size_t _get_size(const Ref<Resource> &p_resource) const;

protected:
	static void _bind_methods();

public:
	MemoryBudget();

	void register_resource(const Ref<Resource> &p_resource);

	auto bytes() const { return _bytes.load(); }

	void process_pending_resources(int p_max = 5);

	void set_budget(const String &p_type, size_t p_budget);
};

} // namespace godot
