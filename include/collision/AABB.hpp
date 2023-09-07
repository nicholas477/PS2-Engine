#pragma once

#include "assert.hpp"
#include "types.hpp"

struct AABB
{
	AABB() = default;
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

	bool is_point_inside(const Vector& Point) const
	{
		return Point.x >= Min.x && Point.y >= Min.y && Point.z >= Min.z && Point.x <= Max.x && Point.y <= Max.y && Point.z <= Max.z;
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

public:
	Vector Min, Max;
};