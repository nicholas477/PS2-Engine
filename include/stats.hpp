#pragma once

#include "engine.hpp"
#include "utils/list.hpp"

#include <string>
#include <map>

namespace Stats
{
enum class scoped_timers : u8 {
	frame = 0,
	tick,
	tick_movement,
	audio,
	draw,
	render_flush,
	render_finish_geom,
	render_vsync_wait,

	MAX = 32
};
template <typename T>
using stats_array_t = std::array<T, static_cast<size_t>(Stats::scoped_timers::MAX)>;

static constexpr stats_array_t<std::pair<scoped_timers, const char*>> scoped_timers_names =
    {{
        {scoped_timers::frame, "frame"},
        {scoped_timers::tick, "tick"},
        {scoped_timers::tick_movement, "tick movement"},
        {scoped_timers::audio, "audio"},
        {scoped_timers::draw, "render/draw"},
        {scoped_timers::render_flush, "render/flush"},
        {scoped_timers::render_finish_geom, "render/finish geometry"},
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

struct ScopedTimer
{
	ScopedTimer(scoped_timers _timer);
	~ScopedTimer();

	u64 start_time;
	scoped_timers timer;
};

struct Timer
{
	Timer()
	{
		start_time = 0;
		end_time   = 0;
		name       = nullptr;
	}

	Timer(const char* in_name)
	    : Timer()
	{
		name = in_name;
	}

	u64 start_time;
	u64 end_time;
	const char* name;

	void start()
	{
		start_time = Engine::get_cpu_ticks();
	}

	void end()
	{
		end_time = Engine::get_cpu_ticks();
	}

	void clear()
	{
		start_time = end_time = 0;
	}

	void print();
};

void init();
void print_timer_stats();
void clear_timer_stats();

void print_elapsed_time(u64 ticks, const char* name);

class Statable: public TIntrusiveLinkedList<Statable>
{
public:
	virtual void clear_stats() {};
	virtual void print_stats() {};

protected:
	Statable(bool add_to_stat_list = true)
	    : TIntrusiveLinkedList<Statable>(add_to_stat_list)
	{
	}
};
} // namespace Stats