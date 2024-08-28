#include "egg-ps2-graphics-lib/vu_programs.hpp"

extern "C" {
//VU_FUNCTIONS(DrawFinish);
VU_FUNCTIONS(xgkick);
VU_FUNCTIONS(VertexColorTextureRenderer);
VU_FUNCTIONS(VertexColorRenderer);
}

namespace egg::ps2::graphics::vu1_programs
{
static u32 draw_finish_program_addr = 0;
u32& get_draw_finish_program_addr()
{
	return draw_finish_program_addr;
}

std::pair<void*, void*> get_draw_finish_program_mem_address()
{
	//return {mVsmStartAddr(DrawFinish), mVsmEndAddr(DrawFinish)};
	return {0, 0};
}

static u32 kick_program_addr = 0;
u32& get_kick_program_addr()
{
	return kick_program_addr;
}

// Returns start/end address of the kick program in memory
std::pair<void*, void*> get_kick_program_mem_address()
{
	return {mVsmStartAddr(xgkick), mVsmEndAddr(xgkick)};
}

static u32 vertex_color_program_addr = 0;
u32& get_vertex_color_program_addr()
{
	return vertex_color_program_addr;
}

std::pair<void*, void*> get_vertex_color_program_mem_address()
{
	return {mVsmStartAddr(VertexColorRenderer), mVsmEndAddr(VertexColorRenderer)};
}

static u32 vertex_color_texture_program_addr = 0;
u32& get_vertex_color_texture_program_addr()
{
	return vertex_color_texture_program_addr;
}

// Returns start/end address of the kick program in memory
std::pair<void*, void*> get_vertex_color_texture_program_mem_address()
{
	return {mVsmStartAddr(VertexColorTextureRenderer), mVsmEndAddr(VertexColorTextureRenderer)};
}

} // namespace egg::ps2::graphics::vu1_programs