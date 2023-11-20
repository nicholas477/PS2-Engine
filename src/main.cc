#include <kernel.h>
#include <stdio.h>

#include "exception_handler.hpp"
#include "engine.hpp"

int main(int argc, char* argv[])
{
	engine::init();

	engine::run();

	// Sleep
	SleepThread();

	// End program.
	return 0;
}
