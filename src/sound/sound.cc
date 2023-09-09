#include "sound/sound.hpp"
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

	audsrv_fmt_t format;
	format.bits     = 16;
	format.freq     = 44100;
	format.channels = 2;
	//audsrv_set_format(&format);

	check(ret == 0);

	printf("loading audiofile...\n");
	std::ifstream audio_file("cdrom:\\SOUNDS\\VINE_BOOM.ADPCM", std::ios::binary);
	check(audio_file.is_open());
	audio_file.seekg(0, std::ios::end);

	u32 adpcmFileSize = audio_file.tellg();
	printf("audio size: %d\n", adpcmFileSize);
	u8 data[adpcmFileSize];
	audio_file.seekg(0, std::ios::beg);

	audio_file.read((char*)data, adpcmFileSize);
	audio_file.close();

	static std::vector<audsrv_adpcm_t> samples;
	for (int i = 0; i < 4; ++i)
	{
		auto& result    = samples.emplace_back();
		result.size     = 0;
		result.buffer   = 0;
		result.loop     = 0;
		result.pitch    = 0;
		result.channels = 0;

		printf("loading audiofile into adpcm thingy...\n");
		if (audsrv_load_adpcm(&result, data, adpcmFileSize))
		{
			printf("AUDSRV returned error string: %s\n", audsrv_get_error_string());
			check(false);
		}

		printf("channels: %d\n", result.channels);
		printf("buffer: %d\n", (int)result.buffer);
		printf("loop: %d\n", result.loop);
	}

	{
		static class test: public tickable
		{
		public:
			test(std::vector<audsrv_adpcm_t>& in_samples)
			    : samples(in_samples)
			{
			}

			virtual void tick(float) override
			{
				if (input::get_paddata() & PAD_TRIANGLE)
				{
					printf("playing audio!\n");
					static int channel = 0;
					audsrv_ch_play_adpcm(channel, &samples[channel]);
					channel++;
					channel = channel % samples.size();
				}
			}

			std::vector<audsrv_adpcm_t>& samples;
		} _test(samples);
	}

	//audsrv_load_adpcm();

	//audsrv_ch_play_adpcm(0, );
}
} // namespace sound