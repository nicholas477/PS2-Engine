#include "exception_handler.hpp"

#include <stdio.h>
#include <tamtypes.h>
#include <kernel.h>
#include <debug.h>
#include <string.h>
#include <stdexcept>

#include "ee_debug.h"

static int exception_handler(struct st_EE_RegFrame*)
{
	printf("Exception!!!!!!!!!!!!!!!!!\n");
	printf("Exception!!!!!!!!!!!!!!!!!\n");
	printf("Exception!!!!!!!!!!!!!!!!!\n");
	printf("Exception!!!!!!!!!!!!!!!!!\n");
	printf("Exception!!!!!!!!!!!!!!!!!\n");
	printf("Exception!!!!!!!!!!!!!!!!!\n");
	printf("Exception!!!!!!!!!!!!!!!!!\n");
	printf("Exception!!!!!!!!!!!!!!!!!\n");
	printf("Exception!!!!!!!!!!!!!!!!!\n");
	printf("Exception!!!!!!!!!!!!!!!!!\n");

	return 0;
}

void install_exception_handler()
{
	printf("Installing exception handler\n");

	printf("ee_dbg_install(3)\n");
	//ee_dbg_install(3);

	// for (int i = 0; i < 16; ++i)
	// {
	// 	//printf("Level 1 exception handler installed: %d\n", i);
	// 	ee_dbg_set_level1_handler(i, &exception_handler);
	// }

	// for (int i = 0; i < 4; ++i)
	// {
	// 	//printf("Level 2 exception handler installed: %d\n", i);
	// 	ee_dbg_set_level2_handler(i, &exception_handler);
	// }

	printf("Exception handler installed\n");
}