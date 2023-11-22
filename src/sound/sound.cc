#include "stats.hpp"
#include "sound/sound.hpp"
#include "threading.hpp"
#include "egg/filesystem.hpp"
#include "tick.hpp"
#include "input.hpp"

#include "egg/assert.hpp"


#include <kernel.h>
#include <stdio.h>
#include <loadfile.h>
#include <audsrv.h>
#include <stdio.h>
#include <fstream>
#include <vector>
#include <fstream>

extern void* _gp;

static int fillbuffer_sema;
static constexpr u16 threadStackSize = 2 * 1024;
static char threadStack[threadStackSize];

static bool song_playing = false;
static bool song_loaded  = false;

static int chunkReadStatus = -1;
static const u16 chunkSize = 4 * 1024;
static char chunk[chunkSize];
static std::ifstream song_file;

static void audioThread();

namespace Sound
{
static void create_audio_thread()
{
	printf("Creating audio thread!\n");

	ee_thread_t audio_thread;
	audio_thread.attr             = 0;
	audio_thread.option           = 0;
	audio_thread.gp_reg           = &_gp;
	audio_thread.func             = reinterpret_cast<void*>(audioThread);
	audio_thread.stack            = threadStack;
	audio_thread.stack_size       = threadStackSize;
	audio_thread.initial_priority = 0x6;
	int audio_thread_id           = CreateThread(&audio_thread);
	check(audio_thread_id >= 0);
	int ret_val = StartThread(audio_thread_id, nullptr);
	printf("start thread ret: %d\n", ret_val);
}

void init()
{
	{
		int ret = SifLoadModule("AUDSRV.IRX"_p.c_str(), 0, nullptr);
		printf("ret: %d\n", ret);
		check(ret >= 0);
	}

	printf("initializing audiosrv...\n");
	int ret = audsrv_init();
	check(ret == 0);

	for (int i = 0; i < 24; ++i)
	{
		audsrv_adpcm_set_volume(i, MAX_VOLUME / 10);
	}

	audsrv_set_volume(50);

	{
		audsrv_fmt_t format;
		format.bits     = 16;
		format.freq     = 22050;
		format.channels = 2;
		int err         = audsrv_set_format(&format);
		if (err != AUDSRV_ERR_NOERROR)
		{
			printf("audsrv_set_format failed with err=%d\n", err);
			check(false);
			exit(-1);
		}
	}

	{
		ee_sema_t sema;
		sema.init_count = 1;
		sema.max_count  = 1;
		sema.option     = 0;
		fillbuffer_sema = CreateSema(&sema);
		check(fillbuffer_sema >= 0);
		int err = audsrv_on_fillbuf(sizeof(chunk), (audsrv_callback_t)iSignalSema, (void*)fillbuffer_sema);
		if (err != AUDSRV_ERR_NOERROR)
		{
			printf("audsrv_on_fillbuf failed with err=%d\n", err);
			check(false);
			exit(-1);
		}
	}

	song_file = std::ifstream("/assets/sounds/white_wa.wav"_p.c_str());
	check(song_file.is_open());
	song_file.seekg(0x30, std::ios_base::beg);
	chunkReadStatus = -1;
	song_playing    = true;

	//create_audio_thread();

	printf("sound initialized!\n");

	// Load the song until the buffer is full
	while (work_song())
	{
	}
}

static void work()
{
	// Switch thread
	Threading::switch_thread();

	work_song();
}

bool work_song()
{
	Stats::scoped_timer audio_timer(Stats::scoped_timers::audio);

	bool did_work = false;
	//printf("work_song()");

	if (!song_playing || !song_file.is_open())
		return did_work;

	if (chunkReadStatus == -1)
	{
		song_file.read(chunk, chunkSize);
		chunkReadStatus = song_file.gcount();
	}

	if (chunkReadStatus > 0)
	{
		//printf("sema: %d\n", fillbuffer_sema);
		int retval = PollSema(fillbuffer_sema);
		//printf("retval for sema: %d\n", retval);
		if (retval == fillbuffer_sema)
		{
			if (audsrv_available() >= chunkReadStatus)
			{
				//audsrv_wait_audio(chunkReadStatus);
				audsrv_play_audio(chunk, chunkReadStatus);

				song_file.read(chunk, chunkSize);
				chunkReadStatus = song_file.gcount();

				did_work = true;
			}
		}
	}

	if (chunkReadStatus < (s32)chunkSize)
		song_playing = false;

	return did_work;
}
} // namespace Sound

static void audioThread()
{
	while (true)
	{
		//printf("hello from sound thread!\n");
		Sound::work();
	}
}