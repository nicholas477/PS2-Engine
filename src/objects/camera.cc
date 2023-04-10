#include <stdio.h>

#include "objects/camera.hpp"
#include "types.hpp"
#include "input.hpp"

static const float dead_zone      = 0.2f;
static const float movement_speed = 150.f;
static const float rotation_speed = M_PI * 0.75f;

static camera _camera;
camera& camera::get()
{
	return _camera;
}

camera::camera()
{
	transform.set_location(Vector(0.f, 0.f, 100.f));
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

	Vector input_vector = Vector();

	input_vector.yaw   = (buttons.rjoy_h - 128.f) / 128.f;
	input_vector.pitch = (buttons.rjoy_v - 128.f) / 128.f;

	const float input_length = input_vector.length();
	if (input_length > dead_zone)
	{
		if (input_length > 1.f)
		{
			input_vector = input_vector.normalize();
		}

		transform.add_rotation(-input_vector * delta_time * rotation_speed);
	}
}

void camera::calculate_movement_input(float delta_time)
{
	const u32 paddata   = input::get_paddata();
	const auto& buttons = input::get_button_status();

	Vector input_vector    = Vector();
	Vector movement_vector = Vector();

	input_vector.x = (buttons.ljoy_h - 128.f) / 128.f;
	input_vector.z = (buttons.ljoy_v - 128.f) / 128.f;

	if (paddata & PAD_CROSS)
	{
		movement_vector.y += 1.f;
	}

	const float input_length = input_vector.length();
	if (input_length > dead_zone)
	{
		if (input_length > 1.f)
		{
			input_vector = input_vector.normalize();
		}

		movement_vector += transform.get_rotation_matrix().transform_vector(input_vector);
	}

	transform.add_location(movement_vector * delta_time * movement_speed);
}