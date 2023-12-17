#include "input/input.hpp"
#include "input/gamepad.hpp"

namespace Input
{
void init()
{
	Gamepad::init();
}

void read_inputs()
{
	Gamepad::read_inputs();
}
} // namespace Input