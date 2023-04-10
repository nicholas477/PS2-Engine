#pragma once
#include "types.h"

class renderable;
class framebuffer_t;
class zbuffer_t;

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