#include "egg-ps2-graphics-lib/vu_programs.hpp"

extern "C" {
VU_FUNCTIONS(DrawFinish);
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
	return {mVsmStartAddr(DrawFinish), mVsmEndAddr(DrawFinish)};
}
} // namespace egg::ps2::graphics::vu1_programs