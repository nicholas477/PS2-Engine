#include <kernel.h>

#include "exception_handler.hpp"
#include "engine.hpp"

int main(int argc, char* argv[])
{
	//install_exception_handler();

	engine::init();

	engine::run();

	// Sleep
	SleepThread();

	// End program.
	return 0;
}
