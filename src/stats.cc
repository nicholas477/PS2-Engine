#include "stats.hpp"
#include "input.hpp"

#include <iostream>
#include <unordered_map>

namespace Stats
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
	printf("%-30s %7d\n", "frame number", Engine::get_frame_counter());
	for (size_t i = 0; i < static_cast<size_t>(Stats::scoped_timers::MAX); ++i)
	{
		if (lookup_stat_timer_name(i) != nullptr)
		{
			u64 timer             = timer_stats[i];
			u64 elapsed_time      = (timer * 1000 * 1000) / Engine::get_cpu_tickrate();
			u32 elapsed_time_ms_i = elapsed_time / 1000;
			u32 elapsed_time_ms_f = elapsed_time % 1000;

			if ((float)elapsed_time > 0.f)
			{
				printf("%-30s %3d.%.3dms, (%f fps)\n", lookup_stat_timer_name(i), elapsed_time_ms_i, elapsed_time_ms_f, (1000.f * 1000.f) / (float)elapsed_time);
			}
			else
			{
				printf("%-30s %3d.%.3dms\n", lookup_stat_timer_name(i), elapsed_time_ms_i, elapsed_time_ms_f);
			}
		}
	}

	for (auto itr = Statable::Itr(); itr; ++itr)
	{
		itr->print_stats();
	}

	printf("----- end timer stats ------\n");
}

void clear_timer_stats()
{
	for (size_t i = 0; i < static_cast<size_t>(Stats::scoped_timers::MAX); ++i)
	{
		timer_stats[i] = 0;
	}
}

ScopedTimer::ScopedTimer(scoped_timers _timer)
    : timer(_timer)
{
	start_time = Engine::get_cpu_ticks();
}

ScopedTimer::~ScopedTimer()
{
	add_timer_stat(timer, Engine::get_cpu_ticks() - start_time);
}

void Timer::print()
{
	print_elapsed_time(end_time - start_time, name);
}

void print_elapsed_time(u64 ticks, const char* name)
{
	u64 elapsed_time      = (ticks * 1000 * 1000) / Engine::get_cpu_tickrate();
	u32 elapsed_time_ms_i = elapsed_time / 1000;
	u32 elapsed_time_ms_f = elapsed_time % 1000;

	if ((float)elapsed_time > 0.f)
	{
		printf("%-30s %3d.%.3dms, (%f fps)\n", name, elapsed_time_ms_i, elapsed_time_ms_f, (1000.f * 1000.f) / (float)elapsed_time);
	}
	else
	{
		printf("%-30s %3d.%.3dms\n", name, elapsed_time_ms_i, elapsed_time_ms_f);
	}
}
} // namespace Stats