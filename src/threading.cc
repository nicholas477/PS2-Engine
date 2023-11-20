#include "threading.hpp"

static timespec tv = {0, 0};

namespace Threading
{
void sleep(const unsigned int& ms)
{
	tv.tv_sec  = ms / 1000;
	tv.tv_nsec = (ms % 1000) * 1000000;
	nanosleep(&tv, nullptr);
}
void sleep(const timespec& t_tv)
{
	nanosleep(&t_tv, nullptr);
}

void switch_thread()
{
	tv.tv_sec  = 0;
	tv.tv_nsec = 500000; // 1/2ms
	nanosleep(&tv, nullptr);
}
} // namespace Threading