#include "renderer/vu1programs/draw_finish.hpp"

static u32 VU1DrawFinish_CodeStart __attribute__((section(".vudata")));
static u32 VU1DrawFinish_CodeEnd __attribute__((section(".vudata")));

draw_finish::draw_finish()
    : VU1Program(&VU1DrawFinish_CodeStart, &VU1DrawFinish_CodeEnd)
{
}

draw_finish& draw_finish::get()
{
	static draw_finish _draw_finish;
	return _draw_finish;
}