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
	if (_graph) {
		_graph->queue_free();
		_graph = nullptr;
	}

	if (_label_2) {
		_label_2->queue_free();
		_label_2 = nullptr;
	}

	if (_label) {
		_label->queue_free();
		_label = nullptr;
	}
}