#pragma once

#include "debugger/debug_receiver.h"
#include "panel.h"
#include <godot_cpp/classes/editor_plugin.hpp>

namespace godot {

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