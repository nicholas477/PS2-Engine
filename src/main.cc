#include <kernel.h>

#include "engine.h"
#include "input.h"

int main(int argc, char *argv[])
{

	engine::init();

	engine::run();

	// Sleep
	SleepThread();

	// End program.
	return 0;
}
