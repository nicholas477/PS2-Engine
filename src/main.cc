#include <kernel.h>
#include <stdio.h>
#include <debug.h>

#include "engine.hpp"
#include "egg/filesystem.hpp"

extern "C" int gdb_stub_main(int argc, char* argv[]);

int main(int argc, char* argv[])
{
	if (Filesystem::get_filesystem_type() == Filesystem::Type::host)
	{
		gdb_stub_main(0, NULL);
	}

	Engine::init(argc, argv);

	Engine::run();

	// Sleep
	SleepThread();

	// End program.
	return 0;
}
