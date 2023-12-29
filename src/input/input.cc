#include "input/input.hpp"
#include "input/gamepad.hpp"
#include "input/keyboard.hpp"

namespace Input
{
void init()
{
	Gamepad::init();
	Keyboard::init();
}

void read_inputs()
{
	Gamepad::read_inputs();
	Keyboard::read_inputs();
}
} // namespace Input