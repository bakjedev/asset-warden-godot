#include "debug_sender.h"
#include <godot_cpp/classes/engine.hpp>

void DebugSender::_bind_methods() {
	ClassDB::bind_method(D_METHOD("send", "data"), &DebugSender::send);
}

Ref<DebugSender> DebugSender::create(const String &p_channel) {
	Ref<DebugSender> ref;
	ref.instantiate();
	ref->initialize(p_channel);
	return ref;
}

void DebugSender::initialize(const String &p_channel) {
	_channel = p_channel;
}

void DebugSender::send(const String &p_id, const Variant &p_data) {
	if (!EngineDebugger::get_singleton()) {
		return;
	}

	String full_id = _channel + ":" + p_id;

	Array data;
	if (p_data.get_type() != Variant::NIL) {
		data.append(p_data);
	}

	EngineDebugger::get_singleton()->send_message(full_id, data);
}