
#pragma once

#include "godot_cpp/classes/v_box_container.hpp"
#include <godot_cpp/classes/label.hpp>

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
} //namespace godot