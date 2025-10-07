#pragma once

#include "godot_cpp/classes/control.hpp"
#include <deque>

namespace godot {
class AssetWardenGraph : public Control {
	GDCLASS(AssetWardenGraph, Control);

private:
	std::deque<Vector2> _points;
	float _x_min = 0.0f;
	float _x_max = 50.0f;
	float _y_min = 0.0f;
	float _y_max = 256.0f;

	int _max_points = 300;

	float _next_pos = 0.0f;

	float grid_interval(float p_range) const;
	Vector2 data_to_screen(const Vector2 &p_point) const;

protected:
	static void _bind_methods();

public:
	AssetWardenGraph();
	~AssetWardenGraph();
	void _draw() override;

	void add_point(float p_value);
	void clear_points();

	void set_scale_x(float p_focus, float p_scale);
	void set_scale_y(float p_focus, float p_scale);
};
} //namespace godot