#include "panel.h"

using namespace godot;

void AssetWardenPanel::_bind_methods() {
	ClassDB::bind_method(D_METHOD("hi"), &AssetWardenPanel::hi);
}

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

void AssetWardenPanel::hi(const String &id, const Array &data) {
	if (id == "request_count") {
		if (_label) {
			int request_count = data[0];
			//_label->set_text(id + String(" Request Count: ") + String::num(request_count, 0));
			_graph->add_point(request_count);
		}
	} else if (id == "queues") {
	}
}