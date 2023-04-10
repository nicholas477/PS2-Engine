#pragma once

#include "math3d.h"
#include "tamtypes.h"
#include "types.hpp"
#include "tick.hpp"
#include "objects/components/transform_component.hpp"

class camera: public tickable
{
public:
	camera();
	virtual void tick(float delta_time) override;

	transform_component transform;

	static camera& get();

protected:
	void calculate_rotation_input(float delta_time);
	void calculate_movement_input(float delta_time);
}; // namespace camera