
#pragma once

#include "godot_cpp/classes/v_box_container.hpp"
#include "graph.h"
#include <godot_cpp/classes/label.hpp>

namespace godot {
class AssetWardenPanel : public VBoxContainer {
	GDCLASS(AssetWardenPanel, VBoxContainer);

private:
	Label *_request_label = nullptr;
	AssetWardenGraph *_request_graph = nullptr;
	Label *_bytes_label = nullptr;
	AssetWardenGraph *_bytes_graph = nullptr;

protected:
	static void _bind_methods() {}

public:
	AssetWardenPanel();
	~AssetWardenPanel();

	auto request_graph() { return _request_graph; }
	auto bytes_graph() { return _bytes_graph; }
	auto request_label() { return _request_label; }
	auto bytes_label() { return _bytes_label; }
};
} //namespace godot