#include "egg-ps2-graphics-lib/draw.hpp"
#include "egg-ps2-graphics-lib/egg-ps2-graphics-lib.hpp"
#include "egg-ps2-graphics-lib/vu_programs.hpp"

#include <packet2.h>
#include <packet2_utils.h>

#include "egg/math_types.hpp"

namespace egg::ps2::graphics
{
void set_fog_color(u8 r, u8 g, u8 b)
{
	prim_t prim;
	prim.type         = PRIM_TRIANGLE;
	prim.shading      = PRIM_SHADE_GOURAUD;
	prim.mapping      = 1;
	prim.fogging      = 0;
	prim.blending     = 1;
	prim.antialiasing = 0;
	prim.mapping_type = PRIM_MAP_ST;
	prim.colorfix     = PRIM_UNFIXED;

	packet2_utils_vu_open_unpack(get_current_vif_packet(), 0, 1);
	{
		packet2_utils_gif_add_set(get_current_vif_packet(), 1);
		packet2_add_2x_s64(get_current_vif_packet(), GS_SET_FOGCOL(r, g, b), GS_REG_FOGCOL);
		packet2_utils_gs_add_prim_giftag(get_current_vif_packet(), &prim, 0,
		                                 ((u64)GIF_REG_RGBAQ) << 0, 1, 0);
	}
	packet2_utils_vu_close_unpack(get_current_vif_packet());
	packet2_utils_vu_add_start_program(get_current_vif_packet(), vu1_programs::get_kick_program_addr());
}
} // namespace egg::ps2::graphics