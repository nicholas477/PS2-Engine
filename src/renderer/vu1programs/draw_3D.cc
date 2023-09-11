#include "renderer/vu1programs/draw_3D.hpp"

static u32 VU1Draw3D_CodeStart __attribute__((section(".vudata")));
static u32 VU1Draw3D_CodeEnd __attribute__((section(".vudata")));

draw_3D::draw_3D()
    : VU1Program(&VU1Draw3D_CodeStart, &VU1Draw3D_CodeEnd)
{
}

draw_3D& draw_3D::get()
{
	static draw_3D _draw_3D;
	return _draw_3D;
}