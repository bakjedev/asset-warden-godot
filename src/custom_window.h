#pragma once

#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/editor_plugin.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/window.hpp>

namespace godot {
class CustomWindow : public EditorPlugin {
	GDCLASS(CustomWindow, EditorPlugin)
private:
	Window *custom_window;

protected:
	static void _bind_methods();

public:
	CustomWindow();
	~CustomWindow();

	void _enter_tree() override;
	void _exit_tree() override;
	void show_custom_window();
	void close_window();
};
} //namespace godot