#include <stdio.h>

#include "objects/camera.hpp"
#include "egg/math_types.hpp"
#include "input/gamepad.hpp"

#include "utils/rendering.hpp"

Camera& Camera::get()
{
	static Camera _camera;
	return _camera;
}

Camera::Camera()
{
	transform.set_location(Vector(0.f, 0.f, 100.f));
	fov = 77.32f; // 90 degrees horizontal FOV @ 4:3
}