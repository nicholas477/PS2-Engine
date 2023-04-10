#pragma once
#include "tick.hpp"

class movement
{
public:
	class transform_component* updated_location_component;
	class transform_component* updated_rotation_component;
};

class flying_movement: public movement, public tickable
{
public:
	virtual void tick(float delta_seconds) override;

protected:
	void calculate_rotation_input(float delta_time);
	void calculate_movement_input(float delta_time);
};

class third_person_movement: public movement, public tickable
{
public:
	virtual void tick(float delta_seconds) override;

protected:
	void calculate_rotation_input(float delta_time);
	void calculate_movement_input(float delta_time);
};