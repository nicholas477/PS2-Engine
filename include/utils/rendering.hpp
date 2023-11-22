#pragma once

#include "renderer/gs.hpp"
#include "collision/AABB.hpp"
#include "collision/plane.hpp"
#include "draw_types.h"

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