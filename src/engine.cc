#include "engine.hpp"
#include "renderer/gs.hpp"
#include "input/input.hpp"
#include "input/gamepad.hpp"
#include "world/world.hpp"
#include "objects/camera.hpp"
#include "objects/teapot.hpp"
#include "audio/audio.hpp"
#include "stats.hpp"
#include "net/net.hpp"
#include "egg/filesystem.hpp"
#include "threading.hpp"

#include "egg/asset.hpp"

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

#ifndef FILESYSTEM_TYPE
#define FILESYSTEM_TYPE Filesystem::Type::cdrom
#endif

namespace Engine
{
static float game_time  = 0.f;
static float tickrate   = 1.f / 59.93f; // ntsc default
static u32 frameCounter = 0;

static void load_asset_manifest()
{
	if (Filesystem::get_filesystem_type() == Filesystem::Type::cdrom)
	{
		size_t manifest_size;
		std::unique_ptr<std::byte[]> asset_manifest_data;

		check(Filesystem::load_file("MANIFEST.ISO"_p, asset_manifest_data, manifest_size));
		Asset::load_asset_table(asset_manifest_data.get(), manifest_size);

		return;
	}
	else if (Filesystem::get_filesystem_type() == Filesystem::Type::host)
	{
		size_t manifest_size;
		std::unique_ptr<std::byte[]> asset_manifest_data;

		check(Filesystem::load_file("MANIFEST.HST"_p, asset_manifest_data, manifest_size));
		Asset::load_asset_table(asset_manifest_data.get(), manifest_size);

		return;
	}

	check(false);
}

static void set_filesystem_type(Filesystem::Type t)
{
	Filesystem::set_filesystem_type(t);

	printf("Using ");
	switch (t)
	{
		case Filesystem::Type::host:
			printf("host");
			break;

		case Filesystem::Type::cdrom:
			printf("cdrom");
			break;

		default:
			break;
	}
	printf(" filesystem type\n");
}

void init(int argc, char* argv[])
{
	Engine::set_filesystem_type(FILESYSTEM_TYPE);

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

	load_asset_manifest();

	sif_load_module("usbd.irx");

	Stats::init();
	Input::init();
	//Filesystem::run_tests();
	Audio::init();
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

			Input::read_inputs();

			tick(tickrate);

			GS::render();
		}

		if (Input::Gamepad::get_paddata() & PAD_SELECT)
		{
			//exit(0);
			//return;
		}

		if (Input::Gamepad::get_paddata() & PAD_START)
		{
			//Sound::set_music_volume(100);
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

bool sif_load_module(const char* module_path)
{
	const char* converted_path = nullptr;
	if (Filesystem::get_filesystem_type() == Filesystem::Type::cdrom)
	{
		converted_path = Filesystem::Path(module_path, true).to_full_filepath();
	}
	else
	{
		converted_path = Filesystem::Path(module_path, false).to_full_filepath();
	}

	int ret = SifLoadModule(converted_path, 0, nullptr);
	printf("ret: %d\n", ret);
	checkf(ret >= 0, converted_path);
	return ret >= 0;
}
} // namespace Engine