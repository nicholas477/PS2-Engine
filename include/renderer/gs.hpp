#pragma once

#include "renderer/renderer_types.hpp"
#include "types.hpp"

#include "draw_buffers.h"
#include <vector>
#include <memory>
#include <functional>
#include <array>

class renderable;

namespace gs
{
struct gs_state
{
	Matrix world_view;
	Matrix view_screen;
	Vector camera_rotation;

	Matrix get_camera_matrix() const;
	Vector get_camera_pos() const;
	Vector get_camera_rotation() const;
};

void init();
void render();
void clear_screen();

// Adds a renderable to a list that is drawn and cleared out every frame
void add_renderable_one_frame(renderable* renderable);
void add_renderable_one_frame(std::function<void(const gs::gs_state&)> func);
} // namespace gs