#pragma once

#include "math3d.h"
#include "tamtypes.h"
#include "types.h"
#include "tick.h"

class camera: public tickable
{
public:
	camera();
	const Vector &get_location() { return location; };
	const Vector &get_rotation() { return rotation; };
	virtual void tick(float deltaTime) override;

protected:
	Vector location;
	Vector rotation;

}; // namespace camera