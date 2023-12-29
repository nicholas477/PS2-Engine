#include "input/keyboard.hpp"

#include <loadfile.h>
#include "egg/assert.hpp"
#include "egg/filesystem.hpp"

#include "libkbd.h"
#include "ps2kbd.h"

static u8 keyboard_status[256];

namespace Input
{
void Keyboard::init()
{

	memset(keyboard_status, sizeof(keyboard_status), 0);

	{
		int ret = SifLoadModule("PS2KBD.IRX"_p.to_full_filepath(), 0, nullptr);
		printf("ret: %d\n", ret);
		checkf(ret >= 0, "PS2KBD.IRX"_p.to_full_filepath());
	}

	// Note: this depends on the USB modules being load (BSD.IRX)
	if (PS2KbdInit() == 0)
	{
		checkf(false, "Failed to initialize");
	}

	PS2KbdSetReadmode(PS2KBD_READMODE_RAW);
}

void Keyboard::read_inputs()
{
	PS2KbdRawKey key;
	while (PS2KbdReadRaw(&key) != 0)
	{
		unsigned char c = (key.key + 'a') - 4;
		//printf("New key: %u, %u, (%u, %c)\n", key.key, key.state, c, c);
		keyboard_status[key.key] = key.state & 0xF;
	}
}

u8 Keyboard::get_key_status(unsigned char key)
{
	const unsigned char actual_key = (key - 'a') + 4;
	//check(actual_key >= 0 && actual_key <= 255);
	return keyboard_status[actual_key];
}

} // namespace Input