#include "panel.h"

using namespace godot;

AssetWardenPanel::AssetWardenPanel() {
	_label = memnew(Label);
	_label->set_text("Load request count");
	add_child(_label);

	_graph = memnew(AssetWardenGraph);
	add_child(_graph);

	// _label_2 = memnew(Label);
	// _label_2->set_text("test");
	// add_child(_label_2);
}

AssetWardenPanel::~AssetWardenPanel() {
	_graph = nullptr;
	_label = nullptr;
	_label_2 = nullptr;
}