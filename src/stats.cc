#include "stats.hpp"
#include "input.hpp"

#include <iostream>
#include <unordered_map>

namespace stats
{
static stats_array_t<u64> timer_stats;

void init()
{
	clear_timer_stats();
}

static void add_timer_stat(scoped_timers timer, u64 elapsed_time)
{
	timer_stats[static_cast<size_t>(timer)] = elapsed_time;
}

void print_timer_stats()
{
	printf("----- timer stats (ms) -----\n");
	printf("%-30s %7d\n", "frame number", engine::get_frame_counter());
	for (size_t i = 0; i < static_cast<size_t>(stats::scoped_timers::MAX); ++i)
	{
		if (lookup_stat_timer_name(i) != nullptr)
		{
			u64 timer             = timer_stats[i];
			u64 elapsed_time      = (timer * 1000 * 1000) / engine::get_cpu_tickrate();
			u32 elapsed_time_ms_i = elapsed_time / 1000;
			u32 elapsed_time_ms_f = elapsed_time % 1000;


			printf("%-30s %3d.%.3dms, (%f fps)\n", lookup_stat_timer_name(i), elapsed_time_ms_i, elapsed_time_ms_f, (1000.f * 1000.f) / (float)elapsed_time);
		}
	}
	printf("----- end timer stats ------\n");
}

void clear_timer_stats()
{
	for (size_t i = 0; i < static_cast<size_t>(stats::scoped_timers::MAX); ++i)
	{
		timer_stats[i] = 0;
	}
}

scoped_timer::scoped_timer(scoped_timers _timer)
    : timer(_timer)
{
	start_time = engine::get_cpu_ticks();
}

scoped_timer::~scoped_timer()
{
	add_timer_stat(timer, engine::get_cpu_ticks() - start_time);
}
} // namespace stats