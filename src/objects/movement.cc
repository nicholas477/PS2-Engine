#include "objects/movement.hpp"
#include "input.hpp"
#include "types.hpp"
#include "objects/components/transform_component.hpp"
#include "generic_math.hpp"

void movement::tick(float delta_time)
{
	Vector last_frame_position = updated_location_component->get_location();

	perform_movement(delta_time);

	velocity = updated_location_component->get_location() - last_frame_position;
	velocity /= delta_time;
}

void flying_movement::perform_movement(float delta_time)
{
	calculate_rotation_input(delta_time);
	calculate_movement_input(delta_time);
}

void flying_movement::calculate_rotation_input(float delta_time)
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

		updated_rotation_component->add_rotation(-input_vector * delta_time * rotation_speed);
	}
}

void flying_movement::calculate_movement_input(float delta_time)
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

		movement_vector += updated_rotation_component->get_rotation_matrix().transform_vector(input_vector);
	}

	updated_location_component->add_location(movement_vector * delta_time * movement_speed);
}



void third_person_movement::perform_movement(float delta_time)
{
	calculate_rotation_input(delta_time);
	calculate_movement_input(delta_time);
}

void third_person_movement::calculate_rotation_input(float delta_time)
{
	//const u32 paddata   = input::get_paddata();
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

		updated_rotation_component->add_rotation(-input_vector * delta_time * rotation_speed);
	}
}

bool third_person_movement::is_braking(const Vector& velocity, const Vector& velocity_target)
{
	Vector inbtwn_vector = velocity_target - velocity;

	return inbtwn_vector.dot(velocity) < 0.f;
}

void third_person_movement::calculate_movement_input(float delta_time)
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

		movement_vector += updated_rotation_component->get_rotation_matrix().transform_vector(input_vector);
		movement_vector.y = 0;
	}

	Vector velocity_target = movement_vector * movement_speed;

	bool isBraking               = is_braking(velocity, velocity_target);
	static bool brakingLastFrame = false;

	if (isBraking)
	{
		updated_location_component->add_location(damperv_exponential(velocity, velocity_target, braking_acceleration, delta_time) * delta_time);
	}
	else
	{
		updated_location_component->add_location(damperv_exponential(velocity, velocity_target, normal_acceleration, delta_time) * delta_time);
	}
}