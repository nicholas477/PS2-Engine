#pragma once

#include "libpad.h"
#include "tamtypes.h"

/*
 * Button bits
 */
#ifndef __INPUT_H__
#define PAD_LEFT 0x0080
#define PAD_DOWN 0x0040
#define PAD_RIGHT 0x0020
#define PAD_UP 0x0010
#define PAD_START 0x0008
#define PAD_R3 0x0004
#define PAD_L3 0x0002
#define PAD_SELECT 0x0001
#define PAD_SQUARE 0x8000
#define PAD_CROSS 0x4000
#define PAD_CIRCLE 0x2000
#define PAD_TRIANGLE 0x1000
#define PAD_R1 0x0800
#define PAD_L1 0x0400
#define PAD_R2 0x0200
#define PAD_L2 0x0100
#endif

namespace input
{
const ::padButtonStatus &get_button_status();
const u32 &get_paddata();
void init();
void read_inputs();
} // namespace input