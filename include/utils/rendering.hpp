#pragma once

#include "egg/math_types.hpp"

#include "renderer/gs.hpp"
#include "collision/AABB.hpp"
#include "collision/plane.hpp"
#include "draw_types.h"

namespace Colors
{
struct Color
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

	constexpr Color(u8 _r, u8 _g, u8 _b, u8 _a = 255)
	    : r(_r)
	    , g(_g)
	    , b(_b)
	    , a(_a)
	{
	}

	constexpr operator color_t() const
	{
		return _color;
	}

	constexpr color_t get_color() const
	{
		return _color;
	}

	constexpr Vector to_vector() const
	{
		return Vector {r / 255.f, g / 255.f, b / 255.f, a / 255.f};
	}
};

constexpr Color red()
{
	return Color(255, 0, 0);
};

constexpr Color green()
{
	return Color(0, 255, 0);
};

constexpr Color blue()
{
	return Color(0, 0, 255);
};

constexpr Color white()
{
	return Color(255, 255, 255);
};

constexpr Color black()
{
	return Color(0, 0, 0);
};
} // namespace Colors