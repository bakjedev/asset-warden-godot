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
	struct ResourceEntry {
		enum SizeState {
			ESTIMATED,
			KNOWN
		};

		ObjectID id;
		StringName type;
		String path;
		// size in bytes
		size_t size;
		size_t estimated_size;
		SizeState size_state;
	};

	Ref<Mutex> _budget_mutex;
	HashMap<uint64_t, ResourceEntry> _resources;

	// asset type and size in bytes
	HashMap<StringName, size_t> _budgets;
	HashMap<StringName, size_t> _sizes;
	HashMap<StringName, size_t> _estimated;

	int _remove_counter = 0;
	const int _remove_interval = 60;

protected:
	static void _bind_methods();

public:
	MemoryBudget();

	void register_resource(const Ref<Resource> &p_resource);

	size_t sizes(const String &p_type = "") const;
	size_t estimated(const String &p_type = "") const;

	void process_pending_resources(int p_max = 5);

	void set_budget(const String &p_type, size_t p_budget);

	bool reserve_budget(const String &p_type, const String &p_path);
	void release_reservation(const String &p_type, const String &p_path);
};

} // namespace godot
