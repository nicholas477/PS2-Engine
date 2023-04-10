#include "engine.hpp"
#include "gs.hpp"
#include "input.hpp"
#include "objects/camera.hpp"
#include "objects/teapot.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "graph.h"

static std::vector<class tickable*>& get_tickables()
{
	static std::vector<class tickable*> tickables;
	return tickables;
}

namespace engine
{
static float tickrate   = 1.f / 59.93f; // ntsc
static u32 frameCounter = 0;

void add_tickable(::tickable* tickable)
{
	get_tickables().push_back(tickable);
}

void init()
{
	input::init();
	gs::init();

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

	static teapot teapot1;
	teapot1.transform.set_location(Vector(30.f));

	static teapot teapot2;
	teapot2.transform.set_location(Vector(-30.f));

	static teapot teapot3;
	teapot3.transform.set_location(Vector(0.f, 20.f));

	static teapot teapot4;
	teapot4.transform.set_location(Vector(0.f, -20.f));
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