#pragma once

#include "math3d.h"
#include "tamtypes.h"
#include "egg/math_types.hpp"
#include "tick.hpp"
#include "objects/components/transform_component.hpp"

class Camera //: public tickable
{
public:
	Camera();

	TransformComponent transform;
	float fov;

	static Camera& get();

	//virtual void tick(float deltaTime) override;
}; // namespace camera