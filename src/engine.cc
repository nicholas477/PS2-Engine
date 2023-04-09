#include "engine.h"
#include "gs.h"
#include "input.h"

#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "camera.h"
#include "graph.h"

static std::vector<class tickable*>& get_tickables()
{
	static std::vector<class tickable*> tickables;
	return tickables;
}

namespace engine
{
camera* _camera;
static float tickrate   = 1.f / 59.93f; // ntsc
static u32 frameCounter = 0;
void init()
{
	input::init();
	gs::init();

	_camera = new camera();
	get_tickables().push_back(_camera);

	printf("Graph mode: ");
	int region = graph_get_region();
	switch (region)
	{
		case GRAPH_MODE_NTSC:
			printf("NTSC");
			tickrate = 1.f / 59.93f;
			break;
		case GRAPH_MODE_PAL:
			printf("PAL");
			tickrate = 1.f / 50.0f;
			break;
		default:
			printf("OTHER");
			break;
	}
	printf(" (%d)\n", region);
}

static void tick(float deltaTime)
{
	for (tickable* _tickable : get_tickables())
	{
		_tickable->tick(deltaTime);
	}
	frameCounter++;
}

void run()
{
	for (;;)
	{
		input::read_inputs();

		if (input::get_paddata() & PAD_START)
		{
			printf("Returning...\n");
			return;
		}

		tick(tickrate);
		gs::render();
	}
}
u32 get_frame_counter() { return frameCounter; }
} // namespace engine