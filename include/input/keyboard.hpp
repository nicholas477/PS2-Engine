#pragma once

#include <cstdint>
#include <array>

#include "libpad.h"
#include "tamtypes.h"

namespace Input::Keyboard
{
enum class KeyStatus : uint8_t {
	none     = 0,
	pressed  = 1,
	holding  = 2,
	released = 3,
};

void init();
void read_inputs();
KeyStatus get_key_status(unsigned char ascii_key);
unsigned char convert_ascii_to_keyboard_key(unsigned char ascii_key);
unsigned char convert_keyboard_key_to_ascii(unsigned char keyboard_key);

// Note: convert ascii keys to keyboard keys before looking up keys in the array
std::array<KeyStatus, 256>& get_all_key_statuses();

// Returns true if a key has been just pressed or is held down
bool is_key_down(unsigned char key);

// Returns true if a key has been just released or wasn't pressed
bool is_key_up(unsigned char key);

bool is_shift_down();
} // namespace Input::Keyboard