#pragma once

#include "godot_cpp/classes/v_box_container.hpp"
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/editor_plugin.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/window.hpp>

namespace godot {
class CustomDock : public Control {
	GDCLASS(CustomDock, Control);

private:
	VBoxContainer *_vbox;
	Label *_label;

protected:
	static void _bind_methods() {}

public:
	CustomDock();
	~CustomDock();

	virtual void _physics_process(double delta) override;
};

class EditorPanel : public EditorPlugin {
	GDCLASS(EditorPanel, EditorPlugin)
private:
	CustomDock *_custom_dock;

protected:
	static void _bind_methods() {}

public:
	EditorPanel();
	~EditorPanel();

	void _enter_tree() override;
	void _exit_tree() override;
};
} //namespace godot