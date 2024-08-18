#include "renderer/vu1/vu_programs.hpp"

static u32 vertex_color_program_addr = 0;
u32& get_vertex_color_program_addr()
{
	return vertex_color_program_addr;
}