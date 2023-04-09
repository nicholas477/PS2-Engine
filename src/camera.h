#pragma once

#include "math3d.h"
#include "tamtypes.h"
#include "types.h"
#include "tick.h"
#include "transform_component.h"

class camera: public tickable
{
public:
	camera();
	virtual void tick(float delta_time) override;

	transform_component transform;

protected:
	void calculate_rotation_input(float delta_time);
	void calculate_movement_input(float delta_time);
}; // namespace camera