#pragma once

#include "assert.hpp"
#include "types.hpp"

struct Plane: public Vector
{
	Plane() = default;
	Plane(const Vector& Base, const Vector& Normal)
	{
		x = Normal.x;
		y = Normal.y;
		z = Normal.z;
		w = Base.dot(Normal);

		check(is_valid());
	}

	bool is_valid() const
	{
		return !(x == 0.f && y == 0.f && z == 0.f && w == 0.f);
	}

	Vector get_normal() const
	{
		return Vector(x, y, z);
	}

	Vector get_origin() const
	{
		return get_normal() * w;
	}
};