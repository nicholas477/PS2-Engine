#pragma once

#include "egg/asset.hpp"

namespace Audio::Sound
{
bool load_sample(Asset::Reference asset);
bool play_sample(Asset::Reference sample);
} // namespace Audio::Sound