#pragma once

#include "math3d.h"
#include "tamtypes.h"
#include "types.hpp"
#include "tick.hpp"
#include "objects/components/transform_component.hpp"

class camera
{
public:
	camera();

	transform_component transform;

	static camera& get();
}; // namespace camera