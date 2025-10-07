// editor_plugin.cpp
#include "editor_plugin.h"

namespace godot {

void AssetWardenEditorPlugin::_on_debug_message(const String &id, const Array &data) {
	if (id == "request_count") {
		int request_count = data[0];
		_panel->graph()->add_point(request_count);
	}
}

void AssetWardenEditorPlugin::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_on_debug_message"), &AssetWardenEditorPlugin::_on_debug_message);
}

AssetWardenEditorPlugin::AssetWardenEditorPlugin() {
	_panel = nullptr;
}

AssetWardenEditorPlugin::~AssetWardenEditorPlugin() {
	if (_panel) {
		_panel->queue_free();
	}
}

void AssetWardenEditorPlugin::_enter_tree() {
	_panel = memnew(AssetWardenPanel);
	add_control_to_bottom_panel(_panel, "bakje panel");

	_debug_receiver = DebugReceiver::create("bakjetest");

	_debug_receiver->on("request_count", Callable(this, "_on_debug_message"));
	_debug_receiver->on("queues", Callable(this, "_on_debug_message"));
	_debug_receiver->on("shutdown", Callable(this, "_on_debug_message"));

	add_debugger_plugin(_debug_receiver->plugin());
}

void AssetWardenEditorPlugin::_exit_tree() {
	if (_debug_receiver.is_valid()) {
		remove_debugger_plugin(_debug_receiver->plugin());
	}

	remove_control_from_bottom_panel(_panel);
	if (_panel) {
		_panel->queue_free();
		_panel = nullptr;
	}
}

} //namespace godot