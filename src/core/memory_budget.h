#pragma once
#include "godot_cpp/classes/mutex.hpp"
#include "godot_cpp/templates/hash_set.hpp"
#include <cstddef>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/resource.hpp>

namespace godot {

class MemoryBudget : public RefCounted {
	GDCLASS(MemoryBudget, RefCounted)

private:
	std::atomic<size_t> _bytes{ 0 };
	HashSet<uint64_t> _cache;
	Ref<Mutex> _cache_mutex;
	std::vector<Ref<Resource>> _pending_resources;

	size_t _get_size(const Ref<Resource> &p_resource) const;

protected:
	static void _bind_methods();

public:
	MemoryBudget();

	void register_resource(const Ref<Resource> &p_resource);

	auto bytes() const { return _bytes.load(); }

	void process_pending_resources(int p_max = 5);
};

} // namespace godot
