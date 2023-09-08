#pragma once

#include "assert.hpp"
#include "types.hpp"

// Represents an infinite plane that bisects space.
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

	// This probably wont be where you think it is
	Vector get_origin() const
	{
		return get_normal() * w;
	}

	float plane_dot(const Vector& P) const
	{
		return x * P.x + y * P.y + z * P.z - w;
	}

	bool line_crosses_plane(const Vector& start, const Vector& end) const
	{
		return (plane_dot(start) < 0.f) != (plane_dot(end) < 0.f);
	}

	void find_best_axis_vectors(Vector& Axis1, Vector& Axis2) const
	{
		const float NX = std::abs(x);
		const float NY = std::abs(y);
		const float NZ = std::abs(z);

		// Find best basis vectors.
		if (NZ > NX && NZ > NY)
			Axis1 = Vector(1, 0, 0);
		else
			Axis1 = Vector(0, 0, 1);

		Vector Tmp = Axis1 - *this * (Axis1.dot(*this));
		Axis1      = Tmp.normalize();
		Axis2      = Axis1.cross(*this);
	}
};

static Vector line_plane_intersection(
    const Vector& Point1,
    const Vector& Point2,
    const Plane& Plane)
{
	return Point1 + (Point2 - Point1) * ((Plane.w - (Point1.dot(Plane))) / ((Point2 - Point1).dot(Plane)));
}