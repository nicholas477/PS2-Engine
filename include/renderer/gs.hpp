#pragma once
#include "draw_buffers.h"
#include "types.hpp"
#include <vector>

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
	Matrix view_world;
	Matrix view_screen;
	framebuffer_t* frame;
	zbuffer_t* zbuffer;
	lightsT lights;
};

void add_renderable(renderable* renderable);
void init();
void render();
} // namespace gs