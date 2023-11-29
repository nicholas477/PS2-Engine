#pragma once

#include <stdexcept>
#include <cstdio>
#include <stdio.h>

#if _EE
#include <kernel.h>
#include "debug.h"

#if 1
#define check(expr)                                                               \
	{                                                                             \
		if (!(expr))                                                              \
		{                                                                         \
			init_scr();                                                           \
			printf("ERROR! Check failed! file: %s, function: %s, line: %d\n",     \
			       __FILE__, __func__, __LINE__);                                 \
			init_scr();                                                           \
			scr_printf("ERROR! Check failed! file: %s, function: %s, line: %d\n", \
			           __FILE__, __func__, __LINE__);                             \
			SleepThread();                                                        \
			throw std::runtime_error("Check failed!\n");                          \
		}                                                                         \
	}
#define checkf(expr, msg)                                                         \
	{                                                                             \
		if (!(expr))                                                              \
		{                                                                         \
			init_scr();                                                           \
			printf("ERROR! Check failed! file: %s, function: %s, line: %d\n",     \
			       __FILE__, __func__, __LINE__);                                 \
			printf("Check msg: %s\n", (msg));                                     \
			init_scr();                                                           \
			scr_printf("ERROR! Check failed! file: %s, function: %s, line: %d\n", \
			           __FILE__, __func__, __LINE__);                             \
			scr_printf("Check msg: %s\n", (msg));                                 \
			SleepThread();                                                        \
			throw std::runtime_error("Check failed!\n");                          \
		}                                                                         \
	}
#else
#define check(expr) \
	{               \
		expr;       \
	}

#define checkf(expr, msg) \
	{               \
		expr;       \
	}
#endif
static void print_stack_trace()
{
	return;

	unsigned int* stackTrace[256];
	int i = 0;

	printf("Stack trace: ---------\n");
	scr_printf("Stack trace: ---------\n");
	ps2GetStackTrace((unsigned int*)&stackTrace, 256);
	for (i = 0; i < 256; ++i)
	{
		if (stackTrace[i] == 0)
			break;

		printf("address %d 0x%08x\n", i, (int)stackTrace[i]);
		scr_printf("address %d 0x%08x\n", i, (int)stackTrace[i]);
	}
	printf("----------------------\n");
	scr_printf("----------------------\n");
}
#else // _EE
#if 1
#define check(expr)                                                                                          \
	{                                                                                                        \
		if (!(expr))                                                                                         \
		{                                                                                                    \
			printf("ERROR! Check failed! file: %s, function: %s, line: %d\n", __FILE__, __func__, __LINE__); \
			throw std::runtime_error("Check failed!\n");                                                     \
		}                                                                                                    \
	}

#define checkf(expr, msg)                                                         \
	{                                                                             \
		if (!(expr))                                                              \
		{                                                                         \
			printf("ERROR! Check failed! file: %s, function: %s, line: %d\n",     \
			       __FILE__, __func__, __LINE__);                                 \
			printf("Check msg: %s\n", (msg));                                     \
			throw std::runtime_error("Check failed!\n");                          \
		}                                                                         \
	}
#else
#define check(expr) \
	{               \
		expr;       \
	}

#define checkf(expr, msg) \
	{                     \
		expr;             \
	}
#endif
#endif