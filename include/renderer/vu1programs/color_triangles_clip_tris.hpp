#pragma once

#include "renderer/vu1_program.hpp"

class color_triangles_clip_tris: public VU1Program
{
public:
	color_triangles_clip_tris();

	virtual std::string getStringName() const { return "color_triangles_clip_tris"; }

	static color_triangles_clip_tris& get();
};