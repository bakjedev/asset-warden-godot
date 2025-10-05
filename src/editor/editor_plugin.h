#pragma once

#include "debugger/debug_receiver.h"
#include "godot_cpp/classes/v_box_container.hpp"
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/editor_plugin.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/window.hpp>

namespace godot {

class AssetWardenPanel : public Control {
	GDCLASS(AssetWardenPanel, Control);

private:
	VBoxContainer *_vbox;
	Label *_label;

protected:
	static void _bind_methods();

public:
	AssetWardenPanel();
	~AssetWardenPanel();

	void hi(const String &id, const Array &data);
};

class AssetWardenEditorPlugin : public EditorPlugin {
	GDCLASS(AssetWardenEditorPlugin, EditorPlugin)
private:
	AssetWardenPanel *_panel;
	Ref<DebugReceiver> _debug_receiver;

protected:
	static void _bind_methods() {}

public:
	AssetWardenEditorPlugin();
	~AssetWardenEditorPlugin();

	void _enter_tree() override;
	void _exit_tree() override;
};

} //namespace godot