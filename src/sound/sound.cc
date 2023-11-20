#include "sound/sound.hpp"
#include "threading.hpp"
#include "utils/filesystem.hpp"
#include "tick.hpp"
#include "input.hpp"

#include "assert.hpp"


#include <kernel.h>
#include <stdio.h>
#include <loadfile.h>
#include <audsrv.h>
#include <stdio.h>
#include <fstream>
#include <vector>
#include <fstream>

static int fillbuffer_sema;
static constexpr u16 threadStackSize = 2 * 1024;
static char threadStack[threadStackSize];

static bool song_playing = false;
static bool song_loaded  = false;

static int chunkReadStatus;
static const u16 chunkSize = 4 * 1024;
static char chunk[chunkSize];
static std::ifstream song_file;

static void audioThread();

namespace sound
{
static void create_audio_thread()
{
	printf("Creating audio thread!\n");

	ee_thread_t audio_thread;
	audio_thread.gp_reg           = &_gp;
	audio_thread.func             = reinterpret_cast<void*>(audioThread);
	audio_thread.stack            = threadStack;
	audio_thread.stack_size       = threadStackSize;
	audio_thread.initial_priority = 0x5;
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

	for (int i = 0; i < 24; ++i)
	{
		audsrv_adpcm_set_volume(i, MAX_VOLUME / 2);
	}
	check(ret == 0);

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
		sema.init_count = 0;
		sema.max_count  = 1;
		sema.option     = 0;
		fillbuffer_sema = CreateSema(&sema);
		int err         = audsrv_on_fillbuf(sizeof(chunk), (audsrv_callback_t)iSignalSema, (void*)fillbuffer_sema);
		if (err != AUDSRV_ERR_NOERROR)
		{
			printf("audsrv_on_fillbuf failed with err=%d\n", err);
			check(false);
			exit(-1);
		}
	}

	//audsrv_play_audio

	song_file = std::ifstream("/assets/sounds/white_waking.wav"_p.c_str());
	song_file.seekg(0x30);
	song_playing = true;

	create_audio_thread();


	//std::vector<std::byte> audio_data;
	// if (Filesystem::load_file("/assets/sounds/vine_boom.adpcm"_p, audio_data))
	// {
	// 	printf("loaded file! nice!\n");
	// }

	// 	static std::vector<audsrv_adpcm_t> samples;
	// 	for (int i = 0; i < 24; ++i)
	// 	{
	// 		auto& result    = samples.emplace_back();
	// 		result.size     = 0;
	// 		result.buffer   = 0;
	// 		result.loop     = 0;
	// 		result.pitch    = 0;
	// 		result.channels = 0;

	// 		printf("loading audiofile into adpcm thingy...\n");
	// 		if (audsrv_load_adpcm(&result, audio_data.data(), audio_data.size()))
	// 		{
	// 			printf("AUDSRV returned error string: %s\n", audsrv_get_error_string());
	// 			check(false);
	// 		}

	// 		printf("channels: %d\n", result.channels);
	// 		printf("buffer: %d\n", (int)result.buffer);
	// 		printf("loop: %d\n", result.loop);
	// 	}

	// 	static class test: public tickable
	// 	{
	// 	public:
	// 		test(std::vector<audsrv_adpcm_t>& in_samples)
	// 		    : samples(in_samples)
	// 		{
	// 		}

	// 		virtual void tick(float) override
	// 		{
	// 			if (input::get_paddata() & PAD_TRIANGLE)
	// 			{
	// 				printf("playing audio!\n");
	// 				static int channel = 0;
	// 				//audsrv_ch_play_adpcm(channel, &samples[channel]);
	// 				channel++;
	// 				channel = channel % samples.size();
	// 			}
	// 		}

	// 		std::vector<audsrv_adpcm_t>& samples;
	// 	} _test(samples);
	// }

	printf("sound initialized!\n");

	//audsrv_load_adpcm();

	//audsrv_ch_play_adpcm(0, );
}

static void work_song()
{
	if (!song_playing || !song_file.is_open())
		return;

	// if (songFinished)
	// {
	// 	if (inLoop)
	// 	{
	// 		for (u32 i = 0; i < getListenersCount(); i++)
	// 			songListeners[i]->listener->onAudioFinish();
	// 		rewindSongToStart();
	// 	}
	// 	else
	// 	{
	// 		stop();
	// 		return;
	// 	}
	// }

	if (chunkReadStatus > 0)
	{
		WaitSema(fillbuffer_sema); // wait until previous chunk wasn't finished
		audsrv_wait_audio(chunkReadStatus);
		audsrv_play_audio(chunk, chunkReadStatus);
		// for (u32 i = 0; i < getListenersCount(); i++)
		// 	songListeners[i]->listener->onAudioTick();
	}

	song_file.read(chunk, chunkSize);
	chunkReadStatus = song_file.gcount();

	if (chunkReadStatus < (s32)chunkSize)
		song_playing = false;
}

static void work()
{
	// Switch thread
	Threading::switch_thread();

	work_song();
}
} // namespace sound

static void audioThread()
{
	while (true)
	{
		sound::work();
	}
}