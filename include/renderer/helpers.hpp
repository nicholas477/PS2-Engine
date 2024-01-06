#pragma once

#include "egg/math_types.hpp"

#include "utils/rendering.hpp"

namespace GS::Helpers
{
// Draw a quad at (screen x, screen y, width, height)
void draw_2d_quad(const Vector& quad_rect, const Colors::Color& color);
} // namespace GS::Helpers