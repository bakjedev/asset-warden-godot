// editor_plugin.cpp
#include "editor_plugin.h"
#include "debug_receiver.h"
#include "godot_cpp/classes/editor_interface.hpp"
#include "godot_cpp/variant/dictionary.hpp"
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/v_box_container.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

void AssetWardenPanel::_bind_methods() {
	ClassDB::bind_method(D_METHOD("hi"), &AssetWardenPanel::hi);
}

AssetWardenPanel::AssetWardenPanel() {
	set_custom_minimum_size({ 0, 200 });
	_vbox = memnew(VBoxContainer);
	add_child(_vbox);

	_label = memnew(Label);
	_label->set_text("not set");
	_vbox->add_child(_label);
}

AssetWardenPanel::~AssetWardenPanel() {
	if (_vbox) {
		_vbox->queue_free();
		_vbox = nullptr;
	}

	if (_label) {
		_label->queue_free();
		_label = nullptr;
	}
}

void AssetWardenPanel::hi(const String &id, const Array &data) {
	if (id == "request_count") {
		if (_label) {
			int request_count = data[0];
			//_label->set_text(id + String(" Request Count: ") + String::num(request_count, 0));
		}
	} else if (id == "queues") {
	}
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