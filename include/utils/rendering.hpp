#pragma once

#include "renderer/gs.hpp"
#include "collision/AABB.hpp"
#include "draw_types.h"

// Draws an AABB in worldspace coordinates
[[nodiscard]] qword_t* draw_aabb(qword_t* q, const gs::gs_state& gs_state, const AABB& aabb, color_t color);
