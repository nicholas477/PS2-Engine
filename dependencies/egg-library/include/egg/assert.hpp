#pragma once

#include <stdexcept>
#include <cstdio>
#include <stdio.h>

#include "debug.h"

#if 1
#define check(expr)                                                                                          \
	{                                                                                                        \
		if (!(expr))                                                                                         \
		{                                                                                                    \
			printf("ERROR! Check failed! file: %s, function: %s, line: %d\n", __FILE__, __func__, __LINE__); \
			printf("Stack trace: ---------\n");                                                              \
			print_stack_trace();                                                                             \
			printf("----------------------\n");                                                              \
			printf("Press start to continue or press select to throw an exception...\n");                    \
			throw std::runtime_error("Check failed!\n");                                                     \
		}                                                                                                    \
	}
#else
#define check(expr) \
	{               \
		expr;       \
	}
#endif

static void print_stack_trace()
{
	unsigned int* stackTrace[256];
	int i = 0;

	ps2GetStackTrace((unsigned int*)&stackTrace, 256);
	for (i = 0; i < 256; ++i)
	{
		if (stackTrace[i] == 0)
			break;

		printf("address %d 0x%08x\n", i, (int)stackTrace[i]);
	}
}