#pragma once

#include "engine.hpp"

#include <string>
#include <map>

namespace stats
{
enum class scoped_timers : u8 {
	frame = 0,
	tick,
	tick_movement,
	draw,
	render_vsync_wait,

	MAX = 32
};
template <typename T>
using stats_array_t = std::array<T, static_cast<size_t>(stats::scoped_timers::MAX)>;

static constexpr stats_array_t<std::pair<scoped_timers, const char*>> scoped_timers_names =
    {{
        {scoped_timers::frame, "frame"},
        {scoped_timers::tick, "tick"},
        {scoped_timers::tick_movement, "tick movement"},
        {scoped_timers::draw, "render/draw"},
        {scoped_timers::render_vsync_wait, "vsync wait (idle time)"},
    }};

static constexpr const char* lookup_stat_timer_name(scoped_timers timer)
{
	for (const std::pair<scoped_timers, const char*>& timer_name : scoped_timers_names)
	{
		if (timer_name.first == timer)
		{
			return timer_name.second;
		}
	}

	return nullptr;
}

static constexpr const char* lookup_stat_timer_name(size_t timer)
{
	return lookup_stat_timer_name(static_cast<scoped_timers>(timer));
}

struct scoped_timer
{
	scoped_timer(scoped_timers _timer);
	~scoped_timer();

	u64 start_time;
	scoped_timers timer;
};

void init();
void print_timer_stats();
void clear_timer_stats();
} // namespace stats