#pragma once

#include "renderer/vu1_program.hpp"

class color_triangles_clip_tris: VU1Program
{
public:
	color_triangles_clip_tris();
	~color_triangles_clip_tris();

	virtual std::string getStringName() { return "color_triangles_clip_tris"; }
};