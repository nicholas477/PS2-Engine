#pragma once

#include "assert.hpp"
#include "types.hpp"

struct AABB
{
	AABB()
	    : Min(Vector::zero)
	    , Max(Vector::zero)
	{
	}

	AABB(const Vector& InMin, const Vector& InMax)
	    : Min(InMin)
	    , Max(InMax)
	{
		check(InMin.x <= InMax.x);
		check(InMin.y <= InMax.y);
		check(InMin.z <= InMax.z);
	}

	static AABB from_position_and_dimensions(const Vector& Position, const Vector& Dimensions)
	{
		check(Dimensions.x >= 0);
		check(Dimensions.y >= 0);
		check(Dimensions.z >= 0);

		return AABB(Position, Position + Dimensions);
	}

	constexpr bool is_point_inside(const Vector& Point) const
	{
		return (Point.x >= Min.x && Point.y >= Min.y && Point.z >= Min.z) && (Point.x <= Max.x && Point.y <= Max.y && Point.z <= Max.z);
	}

	// Returns true if this bounding box intersects another bounding box
	bool intersect(const AABB& Other) const
	{
		check_valid();
		Other.check_valid();

		return Min.x <= Other.Max.x &&
		       Max.x >= Other.Min.x &&
		       Min.y <= Other.Max.y &&
		       Max.y >= Other.Min.y &&
		       Min.z <= Other.Max.z &&
		       Max.z >= Other.Min.z;
	}

	Vector get_dimensions() const
	{
		return Vector(Max.x - Min.x, Max.y - Min.y, Max.z - Min.z);
	}

	void check_valid() const
	{
		check(Min.x <= Max.x);
		check(Min.y <= Max.y);
		check(Min.z <= Max.z);
	}

	AABB transform(const Matrix& matrix) const
	{
		AABB transformed_AABB;
		transformed_AABB.Min = matrix.transform_vector(Min);
		transformed_AABB.Max = matrix.transform_vector(Max);

		if (transformed_AABB.Min.x > transformed_AABB.Max.x)
		{
			std::swap(transformed_AABB.Min.x, transformed_AABB.Max.x);
		}

		if (transformed_AABB.Min.y > transformed_AABB.Max.y)
		{
			std::swap(transformed_AABB.Min.y, transformed_AABB.Max.y);
		}

		if (transformed_AABB.Min.y > transformed_AABB.Max.y)
		{
			std::swap(transformed_AABB.Min.y, transformed_AABB.Max.y);
		}

		transformed_AABB.check_valid();

		return transformed_AABB;
	}

	void get_center_and_half_extents(Vector& center, Vector& half_extents) const
	{
		center       = (Min + Max) / 2.f;
		half_extents = (Max - Min) / 2.f;
	}

	static AABB from_center_and_half_extents(const Vector& center, const Vector& half_extents)
	{
		return AABB(center - half_extents, center + half_extents);
	}

	// Creates an AABB centered on this AABB, but extended by the extents of the other AABB
	AABB minkowski_sum(const AABB& Other) const
	{
		Vector out_center;
		Vector out_extents;
		get_center_and_half_extents(out_center, out_extents);

		Vector other_center;
		Vector other_extents;
		get_center_and_half_extents(other_center, other_extents);

		return from_center_and_half_extents(out_center, out_extents + other_extents);
	}

	enum class AABB_face : u8 {
		x_neg = 0,
		x_pos = 1,
		y_neg = 2,
		y_pos = 3,
		z_neg = 4,
		z_pos = 5
	};

	static Vector AABB_face_to_orientation_vec(AABB_face face)
	{
		switch (face)
		{
			case AABB_face::x_neg:
				return Vector(-1, 0, 0);
			case AABB_face::x_pos:
				return Vector(1, 0, 0);
			case AABB_face::y_neg:
				return Vector(0, -1, 0);
			case AABB_face::y_pos:
				return Vector(0, 1, 0);
			case AABB_face::z_neg:
				return Vector(0, 0 - 1);
			case AABB_face::z_pos:
				return Vector(0, 0, 1);
		}

		check(false);
		return Vector::zero;
	}

	// Get an AABB representing one of the faces of this AABB.
	// Use this for testing if a point is on the surface of an AABB
	AABB get_face_aabb(AABB_face face) const
	{
		switch (face)
		{
			case AABB_face::x_neg:
				return AABB(Min, Vector(Min.x, Max.y, Max.z));
			case AABB_face::x_pos:
				return AABB(Vector(Max.x, Min.y, Min.z), Max);

			case AABB_face::y_neg:
				return AABB(Min, Vector(Max.x, Min.y, Max.z));
			case AABB_face::y_pos:
				return AABB(Vector(Min.x, Max.y, Min.z), Max);

			case AABB_face::z_neg:
				return AABB(Min, Vector(Max.x, Max.y, Min.z));
			case AABB_face::z_pos:
				return AABB(Vector(Min.x, Min.y, Max.z), Max);
		}

		check(false);
		return AABB();
	}

	bool point_on_face(const Vector& point, AABB_face face) const
	{
		const AABB face_aabb = get_face_aabb(face);

		switch (face)
		{
			case AABB_face::x_neg:
				if (!is_nearly_equal(point.x, face_aabb.Min.x, 0.01f))
				{
					return false;
				}
				return point.y >= face_aabb.Min.y && point.z >= face_aabb.Min.z && point.y <= face_aabb.Max.y && point.z <= face_aabb.Max.z;
				break;
			case AABB_face::x_pos:
				if (!is_nearly_equal(point.x, face_aabb.Max.x, 0.01f))
				{
					return false;
				}
				return point.y >= face_aabb.Min.y && point.z >= face_aabb.Min.z && point.y <= face_aabb.Max.y && point.z <= face_aabb.Max.z;
				break;

			case AABB_face::y_neg:
				if (!is_nearly_equal(point.y, face_aabb.Min.y, 0.01f))
				{
					return false;
				}
				return point.x >= face_aabb.Min.x && point.z >= face_aabb.Min.z && point.x <= face_aabb.Max.x && point.z <= face_aabb.Max.z;
			case AABB_face::y_pos:
				if (!is_nearly_equal(point.y, face_aabb.Max.y, 0.01f))
				{
					return false;
				}
				return point.x >= face_aabb.Min.x && point.z >= face_aabb.Min.z && point.x <= face_aabb.Max.x && point.z <= face_aabb.Max.z;

			case AABB_face::z_neg:
				if (!is_nearly_equal(point.z, face_aabb.Max.z, 0.01f))
				{
					return false;
				}
				return point.x >= face_aabb.Min.x && point.y >= face_aabb.Min.y && point.x <= face_aabb.Max.x && point.y <= face_aabb.Max.y;
			case AABB_face::z_pos:
				if (!is_nearly_equal(point.z, face_aabb.Min.z, 0.01f))
				{
					return false;
				}
				return point.x >= face_aabb.Min.x && point.y >= face_aabb.Min.y && point.x <= face_aabb.Max.x && point.y <= face_aabb.Max.y;
		}

		check(false);
		return false;
	}

	// Is a point on any of the faces on the AABB
	bool point_on_any_face(const Vector& point, AABB_face& out_face) const
	{
		for (u8 i = 0; i < 6; ++i)
		{
			out_face = static_cast<AABB_face>(i);

			if (get_face_aabb(out_face).is_point_inside(point))
			{
				return true;
			}
		}
		return false;
	}

public:
	alignas(16) Vector Min;
	alignas(16) Vector Max;
};
