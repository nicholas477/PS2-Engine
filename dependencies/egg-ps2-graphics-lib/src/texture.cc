#include "egg-ps2-graphics-lib/texture.hpp"
#include "egg-ps2-graphics-lib/draw.hpp"
#include "egg-ps2-graphics-lib/types.hpp"
#include "egg-ps2-graphics-lib/egg-ps2-graphics-lib.hpp"
#include "egg-ps2-graphics-lib/vu_programs.hpp"

#include <dma.h>
#include <draw.h>
#include <gs_psm.h>

namespace egg::ps2::graphics
{

texture_descriptor::texture_descriptor()
{
	is_uploaded = false;

	lod.calculation = LOD_USE_K;
	lod.max_level   = 0;
	lod.mag_filter  = LOD_MAG_NEAREST;
	lod.min_filter  = LOD_MIN_NEAREST;
	lod.l           = 0;
	lod.k           = 0;

	clut.storage_mode = CLUT_STORAGE_MODE1;
	clut.start        = 0;
	clut.psm          = GS_PSM_32;
	clut.load_method  = CLUT_NO_LOAD;
	clut.address      = 0;

	t_texbuff.address = 0;
	t_texbuff.width   = 0;
	t_texbuff.psm     = GS_PSM_32;

	t_texbuff.info.width      = 0;
	t_texbuff.info.height     = 0;
	t_texbuff.info.components = TEXTURE_COMPONENTS_RGB;
	t_texbuff.info.function   = TEXTURE_FUNCTION_DECAL;
}


void texture_descriptor::set_width_height(u32 in_width, u32 in_height)
{
	// Check the dimensions are powers of 2
	assert((in_width & (in_width - 1)) == 0);
	assert((in_height & (in_height - 1)) == 0);

	t_texbuff.width = in_width;
	height          = in_height;

	t_texbuff.info.width  = draw_log2(in_width);
	t_texbuff.info.height = draw_log2(in_height);
}

std::pair<u32, u32> texture_descriptor::get_width_height() const
{
	return {t_texbuff.width, height};
}

void upload_texture(texture_descriptor& texture, void* texture_data)
{
	if (texture.is_uploaded)
	{
		return;
	}

	utils::inline_packet2<50> texture_packet(P2_TYPE_NORMAL, P2_MODE_CHAIN, 0);
	packet2_update(texture_packet, draw_texture_transfer(
	                                   texture_packet->next,
	                                   texture_data,
	                                   texture.get_width_height().first,
	                                   texture.get_width_height().second,
	                                   texture.t_texbuff.psm,
	                                   texture.t_texbuff.address,
	                                   texture.t_texbuff.width));
	packet2_update(texture_packet, draw_texture_flush(texture_packet->next));
	dma_channel_send_packet2(texture_packet, DMA_CHANNEL_GIF, 1);
	dma_wait_fast();

	texture.is_uploaded = true;
}

void unload_texture(texture_descriptor& texture)
{
	texture.is_uploaded = false;
}

void set_texture(texture_descriptor& texture)
{
	packet2_utils_vu_open_unpack(get_current_vif_packet(), 0, 1);
	{
		packet2_utils_gif_add_set(get_current_vif_packet(), 1);
		packet2_utils_gs_add_lod(get_current_vif_packet(), &texture.lod);
		packet2_utils_gs_add_texbuff_clut(get_current_vif_packet(), &texture.t_texbuff, &texture.clut);
		packet2_utils_gs_add_prim_giftag(get_current_vif_packet(), &get_empty_prim(), 0,
		                                 ((u64)GIF_REG_RGBAQ) << 0, 1, 0);
	}
	packet2_utils_vu_close_unpack(get_current_vif_packet());
	packet2_utils_vu_add_start_program(get_current_vif_packet(), vu1_programs::get_kick_program_addr());
}

} // namespace egg::ps2::graphics