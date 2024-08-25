#pragma once

#include "tamtypes.h"
#include "draw_primitives.h"

namespace egg::ps2::graphics
{
prim_t& get_empty_prim();
void set_fog_color(u8 r, u8 g, u8 b);
} // namespace egg::ps2::graphics