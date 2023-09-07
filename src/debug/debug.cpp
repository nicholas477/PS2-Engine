#include "debug/debug.hpp"

#include <stdio.h>

#include "debug.h"

void print_stack_trace()
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