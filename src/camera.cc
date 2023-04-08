#include "camera.h"

#include "input.h"

camera::camera()
    : location({0.00f, 0.00f, 100.00f, 1.00f})
    , rotation({0.00f, 0.00f, 0.00f, 1.00f})
{
}

void camera::tick(float deltaTime)
{
	const u32 paddata = input::get_paddata();

	VECTOR movement_vector = {0.f, 0.f, 0.f, 0.f};
	if (paddata & PAD_LEFT)
	{
		movement_vector[0] += -1.f;
	}
	if (paddata & PAD_RIGHT)
	{
		movement_vector[0] += 1.f;
	}
	if (paddata & PAD_UP)
	{
		movement_vector[2] -= 1.f;
	}
	if (paddata & PAD_DOWN)
	{
		movement_vector[2] += 1.f;
	}

	for (int i = 0; i < 4; ++i)
	{
		location[i] += movement_vector[i] * deltaTime * 50.f;
	}
}