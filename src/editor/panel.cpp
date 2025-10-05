#include "panel.h"

using namespace godot;

void AssetWardenPanel::_bind_methods() {
	ClassDB::bind_method(D_METHOD("hi"), &AssetWardenPanel::hi);
}

AssetWardenPanel::AssetWardenPanel() {
	set_custom_minimum_size({ 0, 200 });
	_vbox = memnew(VBoxContainer);
	add_child(_vbox);

	_label = memnew(Label);
	_label->set_text("not set");
	_vbox->add_child(_label);
}

AssetWardenPanel::~AssetWardenPanel() {
	if (_vbox) {
		_vbox->queue_free();
		_vbox = nullptr;
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
		}
	} else if (id == "queues") {
	}
}