#pragma once

#include "renderer/vu1_program.hpp"

class draw_3D: public VU1Program
{
public:
	draw_3D();

	virtual std::string getStringName() const { return "draw_3D"; }

	static draw_3D& get();
};