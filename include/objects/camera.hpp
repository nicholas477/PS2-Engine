#pragma once

#include "math3d.h"
#include "tamtypes.h"
#include "egg/math_types.hpp"
#include "tick.hpp"
#include "objects/components/transform_component.hpp"

class camera //: public tickable
{
public:
	camera();

	transform_component transform;
	float fov;

	static camera& get();

	//virtual void tick(float deltaTime) override;
}; // namespace camera