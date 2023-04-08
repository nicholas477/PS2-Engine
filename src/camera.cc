#include <stdio.h>

#include "camera.h"
#include "types.h"
#include "input.h"

camera::camera()
    : location({0.00f, 0.00f, 100.00f, 1.00f})
    , rotation({0.00f, 0.00f, 0.00f, 1.00f})
{
}

void camera::tick(float deltaTime)
{
	const u32 paddata   = input::get_paddata();
	const auto &buttons = input::get_button_status();

	const float dead_zone = 0.2f;

	Vector input_vector    = Vector();
	Vector movement_vector = Vector();

	input_vector.x = (buttons.ljoy_h - 128.f) / 128.f;
	input_vector.z = (buttons.ljoy_v - 128.f) / 128.f;

	if (input_vector.length() > dead_zone)
	{
		movement_vector = input_vector;
	}

	location += movement_vector * deltaTime * 50.f;
}