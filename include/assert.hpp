#pragma once

#include <stdexcept>
#include <cstdio>
#include "input.hpp"
#include "debug/debug.hpp"

#define check(expr)                                                                                                             \
	{                                                                                                                           \
		if (!(expr))                                                                                                            \
		{                                                                                                                       \
			fprintf(stdout, "ERROR! Check failed! file: %s, function: %s, line: %d\n", __FILE__, __func__, __LINE__);           \
			fprintf(stdout, "Stack trace: ---------\n");                                                                        \
			print_stack_trace();                                                                                                \
			fprintf(stdout, "----------------------\n");                                                                        \
			fprintf(stdout, "Press start to continue or press select to throw an exception...\n");                              \
			std::fflush(stdout);                                                                                                \
			std::fflush(stderr);                                                                                                \
			if (!check_continue())                                                                                              \
			{                                                                                                                   \
				char buffer[256];                                                                                               \
				snprintf(buffer, 256, "ERROR! Check failed! file: %s, function: %s, line: %d\n", __FILE__, __func__, __LINE__); \
				throw std::runtime_error(buffer);                                                                               \
			}                                                                                                                   \
		}                                                                                                                       \
	}


// loops infinitely, returns true if the start button was pressed, false if the select button was pressed
static bool check_continue()
{
	for (;;)
	{
		input::read_inputs();

		if (input::get_paddata() & PAD_START)
		{
			input::read_inputs();
			return true;
		}

		if (input::get_paddata() & PAD_SELECT)
		{
			input::read_inputs();
			return false;
		}
	}
}