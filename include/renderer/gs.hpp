#pragma once
#include "utils/packet.hpp"
#include "draw_buffers.h"
#include "types.hpp"
#include <vector>
#include <memory>
#include <functional>
#include <array>

class renderable;

namespace gs
{
struct lightsT
{
	std::vector<Vector> directions;
	std::vector<Vector> colors;
	enum class type : int {
		ambient     = 0,
		directional = 1
	};
	std::vector<type> types;

	void add_light(const Vector& direction, const Vector& color, type type)
	{
		directions.push_back(direction);
		colors.push_back(color);
		types.push_back(type);
	}

	int count() const
	{
		return directions.size();
	}
};

struct gs_state
{
	Matrix world_view;
	Matrix view_screen;
	framebuffer_t* frame;
	zbuffer_t* zbuffer;
	lightsT lights;
	Vector camera_rotation;

	Matrix get_camera_matrix() const;
	Vector get_camera_pos() const;
	Vector get_camera_rotation() const;

	static std::array<packet2, 2>& get_packets();
	static packet2& get_current_packet();
	static void flip_context();
	static int get_context();
};

void add_renderable(renderable* renderable);
void init();
void render();
void clear_screen();

// Adds a renderable to a list that is drawn and cleared out every frame
void add_renderable_one_frame(renderable* renderable);
void add_renderable_one_frame(std::function<void(const gs::gs_state&)> func);
} // namespace gs