#pragma once
#include "tamtypes.h"

class Tickable;

namespace Engine
{
void init(int argc, char* argv[]);
void run();

/// @brief Returns the current frame number. This is incremented after ticking. The first frame is frame 0.
/// @return
u32 get_frame_counter();

/// @brief Get the game time, in seconds. This is incremented by `delta time` every frame after ticking
/// @return The game time, in seconds
float get_game_time();

/// @brief Gets the real time in seconds since the game started. This is calculated using the system clock. Since it uses floats it's not very accurate for measuring small timeframes. Use `get_cpu_ticks()` for small timeframes.
/// @return
float get_realtime();

/// @brief Gets the number of CPU ticks since the game started. This uses the system clock. If you want to convert this to seconds, use `get_cpu_ticks() / get_cpu_tickrate()`
/// @return
u64 get_cpu_ticks();

/// @brief Gets the number of CPU ticks per second.
/// @return
u64 get_cpu_tickrate();
}; // namespace Engine