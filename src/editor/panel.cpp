#include "panel.h"

using namespace godot;

AssetWardenPanel::AssetWardenPanel() {
	_request_label = memnew(Label);
	_request_label->set_text("Load request count");
	add_child(_request_label);

	_request_graph = memnew(AssetWardenGraph);
	add_child(_request_graph);

	_bytes_label = memnew(Label);
	_bytes_label->set_text("Bytes count");
	add_child(_bytes_label);

	_bytes_graph = memnew(AssetWardenGraph);
	add_child(_bytes_graph);
}

AssetWardenPanel::~AssetWardenPanel() {
	_request_graph = nullptr;
	_request_label = nullptr;
	_bytes_label = nullptr;
	_bytes_graph = nullptr;
}