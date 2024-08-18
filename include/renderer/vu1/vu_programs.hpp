#pragma once

#include <tamtypes.h>

#include "egg-ps2-graphics-lib/vu.hpp"

extern "C" {
VU_FUNCTIONS(VertexColorRenderer);
}

u32& get_vertex_color_program_addr();