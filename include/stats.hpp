#pragma once

#include "engine.hpp"

#include <string>

namespace stats
{
enum class scoped_timers : u8 {
	frame = 0,
	tick,
	render,
	draw,
	movement,

	MAX = 32
};

struct scoped_timer
{
	scoped_timer(scoped_timers _timer);
	~scoped_timer();

	double start_time;
	scoped_timers timer;
};

void init();
void print_timer_stats();
void clear_timer_stats();
void check_stats_input();
} // namespace stats