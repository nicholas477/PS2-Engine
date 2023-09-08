#include <stdio.h>

#include "objects/camera.hpp"
#include "types.hpp"
#include "input.hpp"

#include "utils/rendering.hpp"

static camera _camera;
camera& camera::get()
{
	return _camera;
}

camera::camera()
{
	transform.set_location(Vector(0.f, 0.f, 100.f));
	fov = 50.f;
}

void camera::tick(float deltaTime)
{
	const Vector gizmo_pos = transform.get_matrix().get_location() + (transform.get_forward_vector() * -5) + (transform.get_right_vector() * -2) + (transform.get_up_vector() * -1);
	draw_orientation_gizmo_one_frame(gizmo_pos, 0.5f, true);
}
