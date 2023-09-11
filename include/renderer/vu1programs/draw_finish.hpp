#pragma once

#include "renderer/vu1_program.hpp"

class draw_finish: public VU1Program
{
public:
	draw_finish();

	virtual std::string getStringName() const { return "draw_finish"; }

	static draw_finish& get();
};