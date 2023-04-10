#include <stdio.h>

#include "objects/camera.hpp"
#include "types.hpp"
#include "input.hpp"

static camera _camera;
camera& camera::get()
{
	return _camera;
}

camera::camera()
{
	transform.set_location(Vector(0.f, 0.f, 100.f));
}
