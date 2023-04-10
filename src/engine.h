#pragma once
#include "tamtypes.h"

class tickable;

namespace engine
{
void add_tickable(::tickable* tickable);
void init();
void run();
u32 get_frame_counter();
}; // namespace engine