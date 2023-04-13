#include "stats.hpp"
#include "input.hpp"

#include <unordered_map>

template <typename T>
using stats_array_t = std::array<T, static_cast<size_t>(stats::scoped_timers::MAX)>;

static constexpr stats_array_t<const char*> timer_stats_names = {
    "frame", "tick", "render", "draw", "movement"};

// I would prefer to statically allocate this as a normal array of doubles,
// but doing so crashes the PS2 when you call stats::print() from engine.cc and
// I have literally no idea why
static std::unordered_map<stats::scoped_timers, double> timer_stats;

namespace stats
{
void init()
{
	clear_timer_stats();
}

static void add_timer_stat(scoped_timers timer, double elapsed_time)
{
	timer_stats[timer] = elapsed_time;
}

void print_timer_stats()
{
	printf("----- timer stats -----\n");
	for (size_t i = 0; i < static_cast<size_t>(stats::scoped_timers::MAX); ++i)
	{
		if (timer_stats_names[i] != nullptr)
		{
			const char* name = timer_stats_names[i];
			double timer     = timer_stats[static_cast<stats::scoped_timers>(i)];
			printf("%s (ms): %f\n", name, (float)(timer * 1000.0));
		}
	}
	printf("--- end timer stats ---\n");
}

void clear_timer_stats()
{
	for (size_t i = 0; i < static_cast<size_t>(stats::scoped_timers::MAX); ++i)
	{
		timer_stats[static_cast<stats::scoped_timers>(i)] = 0.0;
	}
}

void check_stats_input()
{
	if (input::get_paddata() & PAD_START)
	{
		stats::print_timer_stats();
	}
}

scoped_timer::scoped_timer(scoped_timers _timer)
    : timer(_timer)
{
	start_time = engine::get_realtime();
}

scoped_timer::~scoped_timer()
{
	add_timer_stat(timer, engine::get_realtime() - start_time);
}
} // namespace stats