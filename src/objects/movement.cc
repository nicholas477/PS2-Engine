#include "objects/movement.hpp"
#include "input.hpp"
#include "egg/math_types.hpp"
#include "objects/components/transform_component.hpp"
#include "objects/components/collision_component.hpp"
#include "generic_math.hpp"
#include "utils/rendering.hpp"

#include "stats.hpp"

#include <algorithm>
#include <cmath>

void movement::tick(float delta_time)
{
	stats::scoped_timer movement_stat(stats::scoped_timers::tick_movement);

	Vector last_frame_position = updated_location_component->get_location();

	perform_movement(delta_time);

	velocity = updated_location_component->get_location() - last_frame_position;
	velocity /= delta_time;
}

static void draw_rejected_hit(const collision_component* collision_component, const Matrix& end_location)
{
	color_t color;
	color.r         = 255;
	color.g         = 0;
	color.b         = 0;
	color.a         = 255.f;
	const AABB aabb = collision_component->get_local_bounds().transform(end_location);

	draw_aabb_one_frame(aabb, color);
}

bool movement::try_move(const Vector& location, Vector& out_sweep_stop)
{
	if (collision_component == nullptr)
	{
		// No collision component, don't need to sweep
		return true;
	}

	const Matrix start_location = updated_location_component->get_matrix();
	const Matrix end_location   = Matrix::from_location_and_rotation(location, Vector::quat_identity);

	hit_result hit = collideable::sweep_collision(*collision_component, start_location, end_location);
	if (hit.hit)
	{
		out_sweep_stop = hit.sweep_location;
		return false;
	}
	return true;
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

	const Vector new_location = updated_location_component->get_location() + movement_vector * delta_time * movement_speed;
	Vector sweep_stop;
	if (try_move(new_location, sweep_stop))
	{
		const Matrix end_location = updated_location_component->get_matrix() + new_location;
		updated_location_component->set_location(end_location.get_location());
	}
	else
	{
		// Hit something
		updated_location_component->set_location(sweep_stop);
	}
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

		target_rotation += -input_vector * delta_time * rotation_speed;
		target_rotation.pitch = normalize_axis(target_rotation.pitch);
		target_rotation.pitch = std::clamp(target_rotation.pitch, degree_to_rad(-85), degree_to_rad(85));
		target_rotation       = target_rotation.clamp_euler_rotation();
	}

	const Vector desired_rot = qinterp_to(updated_rotation_component->get_rotation().euler_to_quat(), target_rotation.euler_to_quat(), delta_time, rotation_lag_speed).quat_to_euler();
	updated_rotation_component->set_rotation(desired_rot);
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

	Vector right_movement_vector = updated_rotation_component->get_right_vector();
	right_movement_vector.y      = 0.f;
	right_movement_vector        = right_movement_vector.normalize();

	Vector forward_movement_vector = updated_rotation_component->get_forward_vector();
	forward_movement_vector.y      = 0.f;
	forward_movement_vector        = forward_movement_vector.normalize();

	input_vector += ((buttons.ljoy_h - 128.f) / 128.f) * right_movement_vector;
	input_vector += ((buttons.ljoy_v - 128.f) / 128.f) * forward_movement_vector;

	if (paddata & PAD_CROSS)
	{
		movement_vector.y += 1.f;
	}

	if (paddata & PAD_CIRCLE)
	{
		movement_vector.y += -1.f;
	}


	const float input_length = input_vector.length();
	if (input_length > dead_zone)
	{
		if (input_length > 1.f)
		{
			input_vector = input_vector.normalize();
		}

		movement_vector += input_vector;
	}

	Vector velocity_target = movement_vector * movement_speed;

	bool isBraking               = is_braking(velocity, velocity_target);
	static bool brakingLastFrame = false;

	Vector location_delta;
	Vector new_location;
	if (isBraking)
	{
		new_location = updated_location_component->get_location() + damperv_exponential(velocity, velocity_target, braking_acceleration, delta_time) * delta_time;
	}
	else
	{
		new_location = updated_location_component->get_location() + damperv_exponential(velocity, velocity_target, normal_acceleration, delta_time) * delta_time;
	}

	Vector sweep_stop;
	if (try_move(new_location, sweep_stop))
	{
		updated_location_component->set_location(new_location);
	}
	else
	{
		velocity = Vector::zero;
		updated_location_component->set_location(sweep_stop);
	}
}