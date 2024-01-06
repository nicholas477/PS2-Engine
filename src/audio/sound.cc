#include "audio/sound.hpp"

#include <audsrv.h>

#include <unordered_map>

namespace Audio::Sound
{
struct Sample
{
public:
	size_t data_size;
	std::unique_ptr<std::byte[]> data;
	audsrv_adpcm_t sample_info;
};

static std::unordered_map<Asset::Reference, Sample> samples;

bool load_sample(Asset::Reference asset)
{
	// If the sample is already loaded
	if (samples.find(asset) != samples.end())
	{
		return true;
	}
	else
	{
		check(Filesystem::file_exists(Asset::lookup_path(asset)));

		Sample& new_sample = samples.emplace(asset, Sample()).first->second;

		check(Filesystem::load_file(Asset::lookup_path(asset), new_sample.data, new_sample.data_size));

		if (audsrv_load_adpcm(&new_sample.sample_info, new_sample.data.get(), new_sample.data_size))
		{
			printf("AUDSRV returned error string: %s\n", audsrv_get_error_string());
			check(false);
			return false;
		}

		printf("channels: %d\n", new_sample.sample_info.channels);
		printf("loop: %d\n", new_sample.sample_info.loop);

		return true;
	}
}

bool play_sample(Asset::Reference sample)
{
	auto sample_itr = samples.find(sample);
	if (sample_itr != samples.end())
	{
		audsrv_ch_play_adpcm(-1, &sample_itr->second.sample_info);
		return true;
	}
	return false;
}
} // namespace Audio::Sound