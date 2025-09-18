// editor_plugin.cpp
#include "custom_window.h"
#include "godot_cpp/classes/editor_interface.hpp"
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/v_box_container.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
void CustomWindow::_bind_methods() {
	ClassDB::bind_method(D_METHOD("show_custom_window"), &CustomWindow::show_custom_window);
	ClassDB::bind_method(D_METHOD("close_window"), &CustomWindow::close_window);
}

CustomWindow::CustomWindow() {
	custom_window = nullptr;
}

CustomWindow::~CustomWindow() {
	if (custom_window) {
		custom_window->queue_free();
	}
}

void CustomWindow::_enter_tree() {
	custom_window = memnew(Window);
	custom_window->set_title("testing");
	custom_window->set_size(Vector2i(400, 300));

	VBoxContainer *vbox = memnew(VBoxContainer);
	custom_window->add_child(vbox);

	Label *label = memnew(Label);
	label->set_text("hi hello.");
	vbox->add_child(label);

	Button *close_btn = memnew(Button);
	close_btn->set_text("close");
	close_btn->connect("pressed", callable_mp(custom_window, &Window::hide));
	vbox->add_child(close_btn);

	get_editor_interface()->get_base_control()->add_child(custom_window);
	custom_window->hide();

	add_tool_menu_item("assetloader menu", callable_mp(this, &CustomWindow::show_custom_window));
}

void CustomWindow::_exit_tree() {
	remove_tool_menu_item("assetloader menu");
	if (custom_window) {
		custom_window->queue_free();
		custom_window = nullptr;
	}
}

void CustomWindow::show_custom_window() {
	if (custom_window) {
		custom_window->popup_centered();
	}
}

void CustomWindow::close_window() {
	if (custom_window) {
		custom_window->hide();
	}
}
} //namespace godot