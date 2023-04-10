#pragma once
#include "draw_buffers.h"
#include "types.hpp"

class renderable;

namespace gs
{
struct gs_state
{
	Matrix view_screen;
	framebuffer_t* frame;
	zbuffer_t* zbuffer;
};

void add_renderable(renderable* renderable);
void init();
void render();
} // namespace gs