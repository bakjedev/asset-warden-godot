#include "debug_receiver.h"
#include "godot_cpp/variant/callable.hpp"
#include <godot_cpp/classes/engine_debugger.hpp>

Ref<DebugReceiver> DebugReceiver::create(const String &p_channel) {
	Ref<DebugReceiver> ref;
	ref.instantiate();
	ref->initialize(p_channel);
	return ref;
}

void DebugReceiver::initialize(const String &p_channel) {
	_plugin.instantiate();
	_plugin->set_channel(p_channel);
}

void DebugReceiver::on(const String &p_id, const Callable &p_callable) {
	_plugin->on(p_id, p_callable);
}

void DebugReceiverPlugin::set_channel(const String &p_channel) {
	_channel = p_channel;
}

void DebugReceiverPlugin::on(const String &p_id, const Callable &p_callable) {
	_callbacks[p_id.utf8().get_data()] = p_callable;
}

bool DebugReceiverPlugin::_has_capture(const String &p_capture) const {
	return p_capture == _channel;
}

bool DebugReceiverPlugin::_capture(const String &p_message, const Array &p_data, int32_t p_session) {
	PackedStringArray parts = p_message.split(":", false, 1);
	if (parts.size() < 2) {
		return false;
	}

	String channel = parts[0];
	String id = parts[1];

	auto it = _callbacks.find(id.utf8().get_data());
	if (it != _callbacks.end()) {
		it->second.call_deferred(id, p_data);
		return true;
	}

	return false;
}