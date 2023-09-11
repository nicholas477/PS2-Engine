#include "renderer/vu1programs/color_triangles_clip_tris.hpp"

extern u32 VU1Prog_Color_Triangles_Clip_Tris_CodeStart __attribute__((section(".vudata")));
extern u32 VU1Prog_Color_Triangles_Clip_Tris_CodeEnd __attribute__((section(".vudata")));

color_triangles_clip_tris::color_triangles_clip_tris()
    : VU1Program(&VU1Prog_Color_Triangles_Clip_Tris_CodeStart, &VU1Prog_Color_Triangles_Clip_Tris_CodeEnd)
{
}

color_triangles_clip_tris& color_triangles_clip_tris::get()
{
	static color_triangles_clip_tris _color_triangles_clip_tris;
	return _color_triangles_clip_tris;
}