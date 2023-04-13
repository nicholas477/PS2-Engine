#include "renderer/vu1programs/color_triangles_clip_verts.hpp"

extern u32 VU1Prog_Color_Triangles_Clip_Verts_CodeStart __attribute__((section(".vudata")));
extern u32 VU1Prog_Color_Triangles_Clip_Verts_CodeEnd __attribute__((section(".vudata")));

color_triangles_clip_verts::color_triangles_clip_verts()
    : VU1Program(&VU1Prog_Color_Triangles_Clip_Verts_CodeStart, &VU1Prog_Color_Triangles_Clip_Verts_CodeEnd)
{
}