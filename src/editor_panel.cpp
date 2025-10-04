// editor_plugin.cpp
#include "editor_panel.h"
#include "debug_receiver.h"
#include "godot_cpp/classes/editor_interface.hpp"
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/v_box_container.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

void CustomDock::_bind_methods() {
	ClassDB::bind_method(D_METHOD("hi"), &CustomDock::hi);
}

CustomDock::CustomDock() {
	set_custom_minimum_size({ 0, 200 });
	_vbox = memnew(VBoxContainer);
	add_child(_vbox);

	_label = memnew(Label);
	_label->set_text("Request count: 0");
	_vbox->add_child(_label);
	set_physics_process(true);
}

CustomDock::~CustomDock() {
	if (_vbox) {
		_vbox->queue_free();
		_vbox = nullptr;
	}

	if (_label) {
		_label->queue_free();
		_label = nullptr;
	}
}

void CustomDock::_physics_process(double) {
	// auto asset_loader = AssetLoader::get_singleton();
	// if (_label && asset_loader) {
	// 	_label->set_text("Request csssount: ");
	// }
}

void CustomDock::hi(const String &id, const Array &data) {
	if (_label) {
		int request_count = data[0];
		_label->set_text(id + String(" Request Count: ") + String::num(request_count, 0));
	}
}

EditorPanel::EditorPanel() {
	_custom_dock = nullptr;
}

EditorPanel::~EditorPanel() {
	if (_custom_dock) {
		_custom_dock->queue_free();
	}
}

void EditorPanel::_enter_tree() {
	_custom_dock = memnew(CustomDock);
	add_control_to_bottom_panel(_custom_dock, "bakje dock");

	_debug_receiver = DebugReceiver::create("bakjetest");

	_debug_receiver->on("request_count", Callable(_custom_dock, "hi"));

	add_debugger_plugin(_debug_receiver->plugin());
}

void EditorPanel::_exit_tree() {
	if (_debug_receiver.is_valid()) {
		remove_debugger_plugin(_debug_receiver->plugin());
	}

	remove_control_from_bottom_panel(_custom_dock);
	if (_custom_dock) {
		_custom_dock->queue_free();
		_custom_dock = nullptr;
	}
}

} //namespace godot