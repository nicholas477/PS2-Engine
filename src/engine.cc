#include "engine.hpp"
#include "renderer/gs.hpp"
#include "input.hpp"
#include "world.hpp"
#include "objects/camera.hpp"
#include "objects/teapot.hpp"
#include "sound/sound.hpp"
#include "stats.hpp"
#include "net/net.hpp"
#include "egg/filesystem.hpp"
#include "threading.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <timer.h>
#include <inttypes.h>
#include <algorithm>

#include <kernel.h>
#include <sifcmd.h>
#include <sifrpc.h>
#include <iopheap.h>
#include <iopcontrol.h>
#include <loadfile.h>
#include <libcdvd-common.h>

#include "graph.h"

namespace Engine
{
static float game_time  = 0.f;
static float tickrate   = 1.f / 59.93f; // ntsc default
static u32 frameCounter = 0;

void init(int argc, char* argv[])
{
	// TODO: figure out why this don't work
	// if (argc == 2 && strcmp(argv[1], "--filesystem=host") == 0)
	// {
	// 	printf("Using host filesystem type\n");
	// 	Filesystem::set_filesystem_type(Filesystem::Type::host);
	// }
	// else
	{
		printf("Using cdrom filesystem type\n");
		Filesystem::set_filesystem_type(Filesystem::Type::cdrom);
	}

	if (Filesystem::get_filesystem_type() != Filesystem::Type::host)
	{
		SifExitIopHeap();
		SifLoadFileExit();
		SifExitRpc();

		SifInitRpc(0);

		while (!SifIopReset("", 0))
			;
		while (!SifIopSync())
			;
	}
	SifInitRpc(0);
	SifLoadFileInit();
	SifInitIopHeap();

	check(SifLoadModule("rom0:LIBSD", 0, NULL) > 0);
	check(SifLoadModule("rom0:SIO2MAN", 0, NULL) > 0);

	if (Filesystem::get_filesystem_type() == Filesystem::Type::cdrom)
	{
		check(SifLoadModule("rom0:CDVDMAN", 0, NULL) > 0);
		check(SifLoadModule("rom0:CDVDFSV", 0, NULL) > 0);

		sceCdInit(SCECdINIT);
		sceCdMmode(SCECdPS2DVD);
	}

	// This initializes the network debugging so do this first
	if (Filesystem::get_filesystem_type() != Filesystem::Type::host)
	{
		//net::init();
	}

	Stats::init();
	Input::init();
	//Filesystem::run_tests();
	Sound::init();
	GS::init();

	printf("Graph mode (region): ");
	// This kinda don't work. We want the refresh rate of the screen and this just gives us PS2 region
	int region = graph_get_region();
	switch (region)
	{
		case GRAPH_MODE_NTSC:
			printf("NTSC");
			//tickrate = 1.f / 59.93f;
			break;
		case GRAPH_MODE_PAL:
			printf("PAL");
			//tickrate = 1.f / 50.0f;
			break;
		default:
			printf("OTHER");
			break;
	}
	tickrate = 1.f / 60.f;
	printf(" (%d)\n", region);

	World::init();
}

static void tick(float deltaTime)
{
	Stats::ScopedTimer tick_timer(Stats::scoped_timers::tick);

	for (auto itr = Tickable::Itr(); itr; ++itr)
	{
		itr->tick(deltaTime);
	}
	frameCounter++;
	game_time += deltaTime;
}

void run()
{
	for (;;)
	{
		{
			Stats::ScopedTimer frame_timer(Stats::scoped_timers::frame);

			//sound::work_song();

			Input::read_inputs();

			//sound::work_song();

			tick(tickrate);

			Sound::work_song();

			GS::render();
		}

		if (Input::get_paddata() & PAD_SELECT)
		{
			//exit(0);
			//return;
		}

		if (Input::get_paddata() & PAD_START)
		{
			Sound::set_music_volume(50);
			Stats::print_timer_stats();
		}
	}
}
u32 get_frame_counter() { return frameCounter; }

float get_game_time()
{
	return game_time;
}

float get_realtime()
{
	return (float)GetTimerSystemTime() / (float)kBUSCLK;
}

u64 get_cpu_ticks()
{
	return GetTimerSystemTime();
}

u64 get_cpu_tickrate()
{
	return kBUSCLK;
}
} // namespace Engine