#pragma once

#include "types.hpp"

#include <cmath>

static float lerpf(float x, float y, float a)
{
	return (1.0f - a) * x + a * y;
}

static Vector lerpv(Vector x, Vector y, float a)
{
	return (1.0f - a) * x + a * y;
}

static float damperf_exponential(float x, float target, float damping, float dt, float ft = 1.0f / 60.0f)
{
	return lerpf(x, target, 1.0f - powf(1.0 / (1.0 - ft * damping), -dt / ft));
}

static Vector damperv_exponential(Vector x, Vector target, float damping, float dt, float ft = 1.0f / 60.0f)
{
	return lerpv(x, target, 1.0f - powf(1.0 / (1.0 - ft * damping), -dt / ft));
}