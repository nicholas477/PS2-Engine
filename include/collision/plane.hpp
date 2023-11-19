#pragma once

#include "assert.hpp"
#include "egg/math_types.hpp"

// Represents an infinite plane that bisects 3d space.
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

	explicit Plane(float x, float y, float z, float w)
	    : Vector(x, y, z, w)
	{
	}

	bool is_valid() const
	{
		return !(x == 0.f && y == 0.f && z == 0.f && w == 0.f);
	}

	Vector get_normal() const
	{
		return Vector(x, y, z);
	}

	// This probably wont be where you think it is, since the position on the surface of the plane doesn't matter
	Vector get_origin() const
	{
		return get_normal() * w;
	}

	/**
	 * Calculates distance between plane and a point.
	 *
	 * @param P The other point.
	 * @return The distance from the plane to the point. 0: Point is on the plane. >0: Point is in front of the plane. <0: Point is behind the plane.
	 */
	float plane_dot(const Vector& P) const
	{
		return (x * P.x + y * P.y + z * P.z) - w;
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
		Axis1      = Tmp.safe_normalize();
		Axis2      = Axis1.cross(*this);
	}

	Plane flip() const
	{
		return Plane(-x, -y, -z, -w);
	}
};

static Vector line_plane_intersection(
    const Vector& Point1,
    const Vector& Point2,
    const Plane& Plane)
{
	return Point1 + (Point2 - Point1) * ((Plane.w - (Point1.dot(Plane))) / ((Point2 - Point1).dot(Plane)));
}