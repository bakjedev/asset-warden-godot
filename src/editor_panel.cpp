// editor_plugin.cpp
#include "editor_panel.h"
#include "asset_loader.h"
#include "godot_cpp/classes/editor_interface.hpp"
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/v_box_container.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

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
	auto asset_loader = AssetLoader::get_singleton();
	if (_label && asset_loader) {
		_label->set_text("Request csssount: ");
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
}

void EditorPanel::_exit_tree() {
	remove_control_from_bottom_panel(_custom_dock);
	if (_custom_dock) {
		_custom_dock->queue_free();
		_custom_dock = nullptr;
	}
}

} //namespace godot