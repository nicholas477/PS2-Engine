#pragma once

#include "renderer/vu1_program.hpp"

class color_triangles_clip_verts: public VU1Program
{
public:
	color_triangles_clip_verts();

	virtual std::string getStringName() const { return "color_triangles_clip_verts"; }
};