#pragma once

#include "egg/math_types.hpp"

#include <algorithm>
#include <cmath>

static float degree_to_rad(float degree)
{
	return (degree / 180.f) * M_PI;
}

// clamps an angle to [0, 2PI]
static float clamp_axis(float angle)
{
	angle = fmod(angle, 2 * M_PI);
	if (angle < 0.f)
	{
		angle += 2 * M_PI;
	}

	return angle;
}

// clamps an angle to (-PI, PI]
float normalize_axis(float angle)
{
	// returns Angle in the range [0,2PI)
	angle = clamp_axis(angle);

	if (angle > M_PI)
	{
		angle -= 2 * M_PI;
	}

	return angle;
}

// Clamps an euler angle
static float clamp_angle(float angle, float min, float max)
{
	const float max_delta         = clamp_axis(min - max) * 0.5f;         // 0..180
	const float range_center      = clamp_axis(max + max_delta);          // 0..360
	const float delta_from_center = normalize_axis(angle - range_center); // -180..180

	// maybe clamp to nearest edge
	if (delta_from_center > max_delta)
	{
		return normalize_axis(range_center + max_delta);
	}
	else if (delta_from_center < -max_delta)
	{
		return normalize_axis(range_center - max_delta);
	}

	// already in range, just return it
	return normalize_axis(angle);
}

static float lerpf(float x, float y, float a)
{
	return (1.0f - a) * x + a * y;
}

static Vector lerpv(const Vector& x, const Vector& y, float a)
{
	return (1.0f - a) * x + a * y;
}

/**
	 * Returns value based on comparand. The main purpose of this function is to avoid
	 * branching based on floating point comparison which can be avoided via compiler
	 * intrinsics.
	 *
	 * Please note that we don't define what happens in the case of NaNs as there might
	 * be platform specific differences.
	 *
	 * @param	Comparand		Comparand the results are based on
	 * @param	ValueGEZero		Return value if Comparand >= 0
	 * @param	ValueLTZero		Return value if Comparand < 0
	 *
	 * @return	ValueGEZero if Comparand >= 0, ValueLTZero otherwise
	 */
static float floatselect(float Comparand, float ValueGEZero, float ValueLTZero)
{
	return Comparand >= 0.f ? ValueGEZero : ValueLTZero;
}

// Slerps from quat x to quat y
static Vector slerp(const Vector& x, const Vector& y, float a)
{
	// Get cosine of angle between quats.
	const float RawCosom =
	    x.x * y.x +
	    x.y * y.y +
	    x.z * y.z +
	    x.w * y.w;

	// Unaligned quats - compensate, results in taking shorter route.
	const float Cosom = floatselect(RawCosom, RawCosom, -RawCosom);

	float Scale0, Scale1;

	if (Cosom < 0.9999f)
	{
		const float Omega  = acos(Cosom);
		const float InvSin = 1.f / sin(Omega);
		Scale0             = sin((1.f - a) * Omega) * InvSin;
		Scale1             = sin(a * Omega) * InvSin;
	}
	else
	{
		// Use linear interpolation.
		Scale0 = 1.0f - a;
		Scale1 = a;
	}

	// In keeping with our flipped Cosom:
	Scale1 = floatselect(RawCosom, Scale1, -Scale1);

	Vector Result;

	Result.x = Scale0 * x.x + Scale1 * y.x;
	Result.y = Scale0 * x.y + Scale1 * y.y;
	Result.z = Scale0 * x.z + Scale1 * y.z;
	Result.w = Scale0 * x.w + Scale1 * y.w;

	return Result.normalize_quat();
}

static float damperf_exponential(float x, float target, float damping, float dt, float ft = 1.0f / 60.0f)
{
	return lerpf(x, target, 1.0f - powf(1.0 / (1.0 - ft * damping), -dt / ft));
}

static Vector damperv_exponential(const Vector& x, const Vector& target, float damping, float dt, float ft = 1.0f / 60.0f)
{
	return lerpv(x, target, 1.0f - powf(1.0 / (1.0 - ft * damping), -dt / ft));
}

static Vector qinterp_to(const Vector& current, const Vector& target, float delta_time, float interp_speed)
{
	// If no interp speed, jump to target value
	if (interp_speed <= 0.f)
	{
		return target;
	}

	// If the values are nearly equal, just return Target and assume we have reached our destination.
	// if (current.Equals(target))
	// {
	// 	return Target;
	// }

	return slerp(current, target, std::clamp(interp_speed * delta_time, 0.f, 1.f));
}