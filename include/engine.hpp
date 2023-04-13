#pragma once
#include "tamtypes.h"

class tickable;

namespace engine
{
void add_tickable(::tickable* tickable);
void init();
void run();
u32 get_frame_counter();
double get_game_time();
float get_realtime();

u64 get_cpu_ticks();
u64 get_cpu_tickrate(); // cpu ticks/second
};                      // namespace engine