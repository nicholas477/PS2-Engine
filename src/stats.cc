#include "stats.hpp"

#include <map>

static std::map<std::string, double>& get_timer_stats()
{
	static std::map<std::string, double> timer_stats;
	return timer_stats;
}

namespace stats
{
void add_timer_stat(const std::string& name, double elapsed_time)
{
	get_timer_stats()[name] = elapsed_time;
}

void print_timer_stats()
{
	printf("----- timer stats -----\n");
	for (std::pair<std::string, double> timers : get_timer_stats())
	{
		printf("%s (ms): %lf\n", timers.first.c_str(), timers.second * 1000.0);
	}
	printf("--- end timer stats ---\n");
}

void clear_timer_stats()
{
	//get_timer_stats().clear();
}
} // namespace stats