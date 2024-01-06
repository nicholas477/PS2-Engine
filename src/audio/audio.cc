#include "stats.hpp"
#include "audio/audio.hpp"
#include "threading.hpp"
#include "egg/filesystem.hpp"
#include "tick.hpp"
#include "input/gamepad.hpp"

#include "egg/assert.hpp"


#include <kernel.h>
#include <stdio.h>
#include <loadfile.h>
#include <audsrv.h>
#include <stdio.h>
#include <fstream>
#include <vector>
#include <fstream>


static bool initialized = false;

namespace Audio
{
void init()
{
	Engine::sif_load_module("audsrv.irx");

	printf("initializing audiosrv...\n");
	int ret = audsrv_init();
	check(ret == 0);

	//check(audsrv_adpcm_init() == 0);

	for (int i = 0; i < 24; ++i)
	{
		audsrv_adpcm_set_volume(i, MAX_VOLUME);
	}

	{
		audsrv_fmt_t format;
		format.bits = 16;
		//format.freq     = 22050;
		format.freq     = 44100;
		format.channels = 2;
		int err         = audsrv_set_format(&format);
		if (err != AUDSRV_ERR_NOERROR)
		{
			printf("audsrv_set_format failed with err=%d\n", err);
			check(false);
			exit(-1);
		}
	}

	printf("Audio initialized!\n");
	initialized = true;
}
} // namespace Audio
