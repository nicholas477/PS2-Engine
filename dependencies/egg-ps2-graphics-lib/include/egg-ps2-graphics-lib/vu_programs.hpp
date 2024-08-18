#pragma once

#include "vu.hpp"

#include <tamtypes.h>
#include <utility>
#include <cstdint>

namespace egg::ps2::graphics::vu1_programs
{
u32& get_draw_finish_program_addr();

// Returns start/end address of the draw_finish program in memory
std::pair<void*, void*> get_draw_finish_program_mem_address();
} // namespace egg::ps2::graphics::vu1_programs