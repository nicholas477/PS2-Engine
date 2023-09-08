#pragma once

#include "renderer/gs.hpp"
#include "collision/AABB.hpp"
#include "collision/plane.hpp"
#include "draw_types.h"

// Draws a line in worldspace coordinates
[[nodiscard]] qword_t* draw_line(qword_t* q, const gs::gs_state& gs_state, const Vector& line_start, const Vector& line_end, color_t color, bool on_top = false);
void draw_line_one_frame(const Vector& line_start, const Vector& line_end, color_t color, bool on_top = false);

// Draws a plane
[[nodiscard]] qword_t* draw_plane(qword_t* q, const gs::gs_state& gs_state, const Plane& plane, float size, color_t color, bool on_top = false);
void draw_plane_one_frame(const Plane& plane, float size, color_t color, bool on_top = false);

// Draws a point in worldspace coordinates
[[nodiscard]] qword_t* draw_point(qword_t* q, const gs::gs_state& gs_state, const Vector& point, float size, color_t color, bool on_top = false);
void draw_point_one_frame(const Vector& point, float size, color_t color, bool on_top = false);

// Draws an AABB in worldspace coordinates
[[nodiscard]] qword_t* draw_aabb(qword_t* q, const gs::gs_state& gs_state, const AABB& aabb, color_t color, bool on_top = false);
void draw_aabb_one_frame(const AABB& aabb, color_t color, bool on_top = false);

// Draws a gizmo where red = x, green = y, blue = z
[[nodiscard]] qword_t* draw_orientation_gizmo(qword_t* q, const gs::gs_state& gs_state, const Vector& pos, float size, bool on_top = false);
void draw_orientation_gizmo_one_frame(const Vector& pos, float size, bool on_top = false);


namespace colors
{
struct color
{
	union
	{
		color_t _color;
		struct
		{
			u8 r;
			u8 g;
			u8 b;
			u8 a;
			float q;
		};
	};

	constexpr color(u8 _r, u8 _g, u8 _b, float _a = 255.f)
	    : r(_r)
	    , g(_g)
	    , b(_b)
	    , a(_a)
	{
	}

	constexpr operator color_t()
	{
		return _color;
	}

	constexpr color_t get_color()
	{
		return _color;
	}
};

constexpr color_t red()
{
	return color(255, 0, 0).get_color();
};

constexpr color_t green()
{
	return color(0, 255, 0).get_color();
};

constexpr color_t blue()
{
	return color(0, 0, 255).get_color();
};

constexpr color_t white()
{
	return color(255, 255, 255).get_color();
};

constexpr color_t black()
{
	return color(0, 0, 0).get_color();
};
} // namespace colors