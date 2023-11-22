#pragma once

#include "renderer/renderer_types.hpp"
#include "egg/math_types.hpp"

#include "draw_buffers.h"
#include <vector>
#include <memory>
#include <functional>
#include <array>

namespace GS
{
struct GSState
{
	Matrix world_view;
	Matrix view_screen;
	Vector camera_rotation;

	Matrix get_camera_matrix() const;
	Vector get_camera_pos() const;
	Vector get_camera_rotation() const;
};

bool has_gs_initialized();
void init();
void render();
void clear_screen();
} // namespace GS