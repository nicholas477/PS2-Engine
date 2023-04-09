#include <stdio.h>

#include "camera.h"
#include "types.h"
#include "input.h"

static const float dead_zone      = 0.2f;
static const float movement_speed = 100.f;
static const float rotation_speed = M_PI / 2.f;

camera::camera()
{
	transform.location = Vector(0.f, 0.f, 100.f, 0.f);
	transform.rotation = Vector(0.f, 0.f, 0.f, 0.f);
}

void camera::tick(float delta_time)
{
	calculate_rotation_input(delta_time);
	calculate_movement_input(delta_time);
}

void camera::calculate_rotation_input(float delta_time)
{
	const u32 paddata   = input::get_paddata();
	const auto& buttons = input::get_button_status();

	Vector input_vector    = Vector();
	Vector rotation_vector = Vector();

	input_vector.yaw   = (buttons.rjoy_h - 128.f) / 128.f;
	input_vector.pitch = (buttons.rjoy_v - 128.f) / 128.f;

	if (input_vector.length() > dead_zone)
	{
		rotation_vector = input_vector;

		transform.rotation += -rotation_vector * delta_time * rotation_speed;
		transform.rotation = transform.rotation.normalize_rotation();

		printf("Rotation vector: %s", transform.rotation.to_string().c_str());
	}
}

void camera::calculate_movement_input(float delta_time)
{
	const u32 paddata   = input::get_paddata();
	const auto& buttons = input::get_button_status();

	Vector input_vector    = Vector();
	Vector movement_vector = Vector(0.f, 0.f, 0.f, 1.f);

	input_vector += transform.get_right_vector() * ((buttons.ljoy_h - 128.f) / 128.f);
	input_vector += transform.get_forward_vector() * ((buttons.ljoy_v - 128.f) / 128.f);

	if (input_vector.length() > dead_zone)
	{
		//printf("Rotation vector: %s", transform.rotation.to_string().c_str());
		//printf("Forward vector: %s", transform.get_forward_vector().to_string().c_str());
		//printf("Right vector: %s", transform.get_right_vector().to_string().c_str());

		movement_vector = input_vector;
		transform.location += movement_vector * delta_time * movement_speed;
	}
}