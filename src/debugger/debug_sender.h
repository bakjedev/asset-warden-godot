#pragma once

#include "godot_cpp/classes/ref_counted.hpp"
#include <godot_cpp/classes/engine_debugger.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

class DebugSender : public RefCounted {
	GDCLASS(DebugSender, RefCounted)

private:
	String _channel;

protected:
	static void _bind_methods();

public:
	DebugSender() = default;
	static Ref<DebugSender> create(const String &p_channel = "test");

	void initialize(const String &p_channel);

	void send(const String &p_id, const Variant &p_data = Variant::NIL);
};