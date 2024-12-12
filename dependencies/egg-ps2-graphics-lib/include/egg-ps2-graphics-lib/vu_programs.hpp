#pragma once

#include "vu.hpp"

#include <tamtypes.h>
#include <utility>
#include <cstdint>

namespace egg::ps2::graphics
{
struct vu_program
{
	vu_program()
	{
		address.first   = 0;
		address.second  = 0;
		program_address = 0;
		is_loaded       = false;
	}

	// program start/end address in EE memory
	std::pair<void*, void*> address;

	// program address in VU memory
	// from 0-1023
	u32 program_address;

	bool is_loaded;

	void load_program();
};
} // namespace egg::ps2::graphics

#define DEFINE_VU_PROGRAM(name, vu_program_name)                           \
	extern "C" {                                                           \
	VU_FUNCTIONS(vu_program_name);                                         \
	}                                                                      \
                                                                           \
	static struct vu_program_##name: public egg::ps2::graphics::vu_program \
	{                                                                      \
		vu_program_##name()                                                \
		{                                                                  \
			address.first  = mVsmStartAddr(vu_program_name);               \
			address.second = mVsmEndAddr(vu_program_name);                 \
		}                                                                  \
                                                                           \
	} name;                                                                \
                                                                           \
	egg::ps2::graphics::vu_program& get_##name()                           \
	{                                                                      \
		return name;                                                       \
	}

#define DECLARE_VU_PROGRAM(name) \
	egg::ps2::graphics::vu_program& get_##name();

namespace egg::ps2::graphics::vu1_programs
{
DECLARE_VU_PROGRAM(xgkick);
DECLARE_VU_PROGRAM(vertex_color_texture_renderer);
DECLARE_VU_PROGRAM(vertex_color_renderer);
//DECLARE_VU_PROGRAM(project);
} // namespace egg::ps2::graphics::vu1_programs
