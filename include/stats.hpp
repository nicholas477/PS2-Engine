#pragma once

#include "engine.hpp"

#include <string>

namespace stats
{
void add_timer_stat(const std::string& name, double elapsed_time);
void print_timer_stats();
void clear_timer_stats();

struct scoped_timer
{
	scoped_timer(const std::string& _name);

	~scoped_timer();

	std::string name;
	double start_time;
};
} // namespace stats