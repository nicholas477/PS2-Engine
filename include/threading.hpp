#pragma once

#include <time.h>

namespace Threading
{
void sleep(const unsigned int& ms);
void sleep(const timespec& tv);

void switch_thread();
} // namespace Threading