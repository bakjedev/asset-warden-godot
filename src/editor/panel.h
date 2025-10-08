
#pragma once

#include "godot_cpp/classes/v_box_container.hpp"
#include "graph.h"
#include <godot_cpp/classes/label.hpp>

namespace godot {
class AssetWardenPanel : public VBoxContainer {
	GDCLASS(AssetWardenPanel, VBoxContainer);

private:
	Label *_label = nullptr;
	Label *_label_2 = nullptr;
	AssetWardenGraph *_graph = nullptr;

protected:
	static void _bind_methods() {}

public:
	AssetWardenPanel();
	~AssetWardenPanel();

	auto graph() { return _graph; }
	auto label() { return _label; }
};
} //namespace godot