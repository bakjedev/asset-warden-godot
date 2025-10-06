#include "graph.h"
#include "godot_cpp/classes/control.hpp"
#include "godot_cpp/classes/rendering_server.hpp"

using namespace godot;

float AssetWardenGraph::grid_interval(float p_range) const {
	float interval = p_range / 10.0f;

	float magnitude = std::pow(10.0f, std::floor(std::log10(interval)));
	float normalized = interval / magnitude;

	float result;
	if (normalized <= 1.0f) {
		result = magnitude;
	} else if (normalized <= 2.0f) {
		result = 2.0f * magnitude;
	} else if (normalized <= 5.0f) {
		result = 5.0f * magnitude;
	} else {
		result = 10.0f * magnitude;
	}

	return result;
}

Vector2 AssetWardenGraph::data_to_screen(const Vector2 &p_point) const {
	Vector2 size = get_size();

	float norm_x = (p_point.x - _x_min) / (_x_max - _x_min);
	float norm_y = (p_point.y - _y_min) / (_y_max - _y_min);

	float screen_x = norm_x * size.x;
	float screen_y = (1.0f - norm_y) * size.y;

	return Vector2(screen_x, screen_y);
}

void AssetWardenGraph::_bind_methods() {
}

AssetWardenGraph::AssetWardenGraph() {
	set_h_size_flags(Control::SIZE_EXPAND_FILL);
	set_v_size_flags(Control::SIZE_EXPAND_FILL);

	_points.emplace_back(0.0f, 100.0f);
}

AssetWardenGraph::~AssetWardenGraph() {
}

void AssetWardenGraph::_draw() {
	draw_set_transform_matrix(Transform2D());
	RenderingServer::get_singleton()->canvas_item_set_clip(get_canvas_item(), true);

	Vector2 size = get_size();
	draw_rect({ {}, size }, { 0.8f, 0.8f, 0.8f });

	float x_interval = grid_interval(_x_max - _x_min);
	float y_interval = grid_interval(_y_max - _y_min);

	Color grid_color(0.3f, 0.3f, 0.3f, 0.5f);
	Color axis_color(0.5f, 0.5f, 0.5f, 1.0f);

	for (float x = std::ceil(_x_min / x_interval) * x_interval; x <= _x_max; x += x_interval) {
		Vector2 screen_pos = data_to_screen(Vector2(x, _y_min));
		float screen_x = screen_pos.x;

		Color line_color = (std::abs(x) < 0.001f) ? axis_color : grid_color;
		float line_width = (std::abs(x) < 0.001f) ? 2.0f : 1.0f;

		draw_line(Vector2(screen_x, 0), Vector2(screen_x, size.y), line_color, line_width);
	}

	for (float y = std::ceil(_y_min / y_interval) * y_interval; y <= _y_max; y += y_interval) {
		Vector2 screen_pos = data_to_screen(Vector2(_x_min, y));
		float screen_y = screen_pos.y;

		Color line_color = (std::abs(y) < 0.001f) ? axis_color : grid_color;
		float line_width = (std::abs(y) < 0.001f) ? 2.0f : 1.0f;

		draw_line(Vector2(0, screen_y), Vector2(size.x, screen_y), line_color, line_width);
	}

	Color line_color(0.2f, 0.2f, 0.8f, 1.0f);
	if (_points.size() >= 2) {
		for (size_t i = 1; i < _points.size(); i++) {
			Vector2 from = data_to_screen(_points[i - 1]);
			Vector2 to = data_to_screen(_points[i]);

			draw_line(from, to, line_color, 2.0f);
			draw_circle(to, 3.0f, line_color);
		}

		if (!_points.empty()) {
			Vector2 first = data_to_screen(_points[0]);
			draw_circle(first, 3.0f, line_color);
		}
	} else if (!_points.empty()) {
		Vector2 first = data_to_screen(_points[0]);
		draw_circle(first, 3.0f, line_color);
	}
}

void AssetWardenGraph::add_point(float p_value) {
	_points.emplace_back(_next_pos, p_value);
	_next_pos += 0.1f;
	set_scale_x(_next_pos, 50.0f);
}

void AssetWardenGraph::set_scale_x(float p_focus, float p_scale) {
	_x_min = p_focus - p_scale / 2.0f;
	_x_max = p_focus + p_scale / 2.0f;
	queue_redraw();
}

void AssetWardenGraph::set_scale_y(float p_focus, float p_scale) {
	_y_min = p_focus - p_scale / 2.0f;
	_y_max = p_focus + p_scale / 2.0f;
	queue_redraw();
}