#include <kernel.h>
#include <stdio.h>

#include "engine.hpp"

int main(int argc, char* argv[])
{
	Engine::init(argc, argv);

	Engine::run();

	// Sleep
	SleepThread();

	// End program.
	return 0;
}
