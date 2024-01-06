#include "input/keyboard.hpp"

#include "engine.hpp"

#include <loadfile.h>
#include "egg/assert.hpp"
#include "egg/filesystem.hpp"

#include "libkbd.h"
#include "ps2kbd.h"

namespace Input
{
static bool initialized = false;
static std::array<Keyboard::KeyStatus, 256> keyboard_status;

std::array<Keyboard::KeyStatus, 256>& Keyboard::get_all_key_statuses()
{
	return keyboard_status;
}

void Keyboard::init()
{
	memset(keyboard_status.data(), 0, sizeof(keyboard_status));

	Engine::sif_load_module("ps2kbd.irx");

	// Note: this depends on the USB modules being load (BSD.IRX)
	if (PS2KbdInit() == 0)
	{
		//checkf(false, "Failed to initialize");
		printf("Failed to initialize keyboard!\n");
		initialized = false;
	}
	else
	{
		PS2KbdSetReadmode(PS2KBD_READMODE_RAW);
		PS2KbdSetBlockingMode(false);

		initialized = true;
	}
}

void Keyboard::read_inputs()
{
	if (!initialized)
	{
		return;
	}

	for (int i = 0; i < 256; ++i)
	{
		if (keyboard_status[i] == KeyStatus::pressed)
		{
			keyboard_status[i] = KeyStatus::holding;
		}
		else if (keyboard_status[i] == KeyStatus::released)
		{
			keyboard_status[i] = KeyStatus::none;
		}
	}

	PS2KbdRawKey key;
	while (PS2KbdReadRaw(&key) != 0)
	{
		unsigned char c = (key.key + 'a') - 4;
		printf("New key: %u, %u, (%u, %c)\n", key.key, key.state, c, c);
		if (key.state & 1)
		{
			keyboard_status[key.key] = KeyStatus::pressed;
		}
		else
		{
			keyboard_status[key.key] = KeyStatus::released;
		}
	}
}

Keyboard::KeyStatus Keyboard::get_key_status(unsigned char ascii_key)
{
	return keyboard_status[convert_ascii_to_keyboard_key(ascii_key)];
}

unsigned char Keyboard::convert_ascii_to_keyboard_key(unsigned char ascii_key)
{
	return (ascii_key - 'a') + 4;
}

unsigned char Keyboard::convert_keyboard_key_to_ascii(unsigned char keyboard_key)
{
	return (keyboard_key + 'a') - 4;
}

bool Keyboard::is_key_down(unsigned char key)
{
	const KeyStatus k = Keyboard::get_key_status(key);
	return k == KeyStatus::holding || k == KeyStatus::pressed;
}

bool Keyboard::is_key_up(unsigned char key)
{
	const KeyStatus k = Keyboard::get_key_status(key);
	return k == KeyStatus::released || k == KeyStatus::none;
}

bool Keyboard::is_shift_down()
{
	const KeyStatus k = keyboard_status[225];
	return k == KeyStatus::holding || k == KeyStatus::pressed;
}

} // namespace Input