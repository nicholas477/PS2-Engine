#include <kernel.h>
#include <stdio.h>

#include <debug.h>
#include <loadfile.h>
#include <smem.h>
#include <smod.h>
#include <sifcmd.h>
#include <sifrpc.h>
#include <iopheap.h>
#include <iopcontrol.h>
#include <sbv_patches.h>
#include <libcdvd-common.h>

//#include "exception_handler.hpp"
#include "engine.hpp"

//DISABLE_PATCHED_FUNCTIONS();

int main(int argc, char* argv[])
{
	engine::init();

	engine::run();

	// Sleep
	SleepThread();

	// End program.
	return 0;
}
