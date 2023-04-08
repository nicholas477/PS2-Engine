#pragma once

#include "math3d.h"
#include "tamtypes.h"
#include "tick.h"

class camera: public tickable
{
public:
	camera();
	const VECTOR &get_location() { return location; };
	const VECTOR &get_rotation() { return rotation; };
	virtual void tick(float deltaTime) override;

protected:
	VECTOR location;
	VECTOR rotation;

}; // namespace camera