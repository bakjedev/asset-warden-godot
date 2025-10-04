#pragma once

#include "godot_cpp/classes/editor_debugger_plugin.hpp"
#include "godot_cpp/classes/ref_counted.hpp"
#include "godot_cpp/variant/string.hpp"
#include <godot_cpp/variant/callable.hpp>
#include <unordered_map>

using namespace godot;

class DebugReceiverPlugin;

class DebugReceiver : public RefCounted {
	GDCLASS(DebugReceiver, RefCounted);

private:
	Ref<DebugReceiverPlugin> _plugin;

protected:
	static void _bind_methods() {}

public:
	static Ref<DebugReceiver> create(const String &p_channel = "test");

	void initialize(const String &p_channel);

	void on(const String &p_id, const Callable &p_callable);

	Ref<DebugReceiverPlugin> plugin() const { return _plugin; }
};

class DebugReceiverPlugin : public EditorDebuggerPlugin {
	GDCLASS(DebugReceiverPlugin, EditorDebuggerPlugin);

private:
protected:
	static void _bind_methods() {}
	String _channel;
	std::unordered_map<std::string, Callable> _callbacks;

public:
	void set_channel(const String &p_channel);

	void on(const String &p_id, const Callable &p_callable);

	virtual bool _has_capture(const String &p_capture) const override;
	virtual bool _capture(const String &p_message, const Array &p_data, int32_t p_session) override;
};