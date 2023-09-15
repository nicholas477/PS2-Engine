#include "sound/sound.hpp"
#include "utils/filesystem.hpp"
#include "tick.hpp"
#include "input.hpp"

#include "assert.hpp"


#include <kernel.h>
#include <loadfile.h>
#include <audsrv.h>
#include <stdio.h>
#include <fstream>
#include <vector>

namespace sound
{
void init()
{
	{
		int ret = SifLoadModule("cdrom0:\\AUDSRV.IRX", 0, nullptr);
		printf("ret: %d\n", ret);
		check(ret >= 0);
	}

	printf("initializing audiosrv...\n");
	int ret = audsrv_init();

	for (int i = 0; i < 24; ++i)
	{
		audsrv_adpcm_set_volume(i, MAX_VOLUME);
	}
	check(ret == 0);

	printf("loading audiofile...\n");
	std::vector<std::byte> audio_data;
	if (filesystem::load_file("assets/sounds/vine_boom.adpcm", audio_data))
	{
		printf("loaded file! nice!\n");
	}

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
} // namespace sound