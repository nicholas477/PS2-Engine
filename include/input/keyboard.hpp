#pragma once

#include "libpad.h"
#include "tamtypes.h"

namespace Input::Keyboard
{
void init();
void read_inputs();
u8 get_key_status(unsigned char key);
} // namespace Input::Keyboard