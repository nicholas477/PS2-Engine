#include <kernel.h>
#include <stdio.h>
#include <debug.h>

#include "engine.hpp"
#include "egg/debug.hpp"
#include "egg/filesystem.hpp"

int main(int argc, char** argv)
{
	Debug::init(argc, argv);

	Engine::init(argc, argv);

	Engine::run();

	// Sleep
	SleepThread();

	// End program.
	return 0;
}
