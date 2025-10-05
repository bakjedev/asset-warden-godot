// editor_plugin.cpp
#include "editor_plugin.h"

namespace godot {

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

	_debug_receiver->on("request_count", Callable(_panel, "hi"));
	_debug_receiver->on("queues", Callable(_panel, "hi"));

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