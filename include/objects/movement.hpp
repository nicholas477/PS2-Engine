#pragma once
#include "tick.hpp"
#include "types.hpp"

class movement: public tickable
{
public:
	class collision_component* collision_component;
	class transform_component* updated_location_component;
	class transform_component* updated_rotation_component;

	virtual void tick(float delta_seconds) override final;

	// Sweeps movement to the new location, returning false if it hit something.
	virtual bool try_move(const Vector& location);

protected:
	virtual void perform_movement(float delta_seconds) {};

	Vector velocity;

	// Input dead zone on the controller
	float dead_zone = 0.2f;

	float rotation_speed = M_PI * 0.8f;
};

class flying_movement: public movement
{
public:
	virtual void perform_movement(float delta_seconds) override;

	float movement_speed = 150.f;

protected:
	void calculate_rotation_input(float delta_time);
	void calculate_movement_input(float delta_time);
};

class third_person_movement: public movement
{
public:
	virtual void perform_movement(float delta_seconds) override;
	static bool is_braking(const Vector& velocity, const Vector& velocity_target);

	float movement_speed       = 150.f;
	float normal_acceleration  = 5.0f;
	float braking_acceleration = 7.5f;

	float rotation_lag_speed = 17.5f;

	Vector target_rotation;
	// Previously desired rotation (quaternion)
	Vector previously_desired_rot;

protected:
	void calculate_rotation_input(float delta_time);
	void calculate_movement_input(float delta_time);
};