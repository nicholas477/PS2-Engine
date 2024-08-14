#include "egg-ps2-graphics-lib/egg-ps2-graphics-lib.hpp"

#include <kernel.h>
#include <malloc.h>
#include <tamtypes.h>
#include <gs_psm.h>
#include <dma.h>
#include <packet2.h>
#include <packet2_utils.h>
#include <graph.h>
#include <draw.h>

#include "egg/math_types.hpp"

namespace
{
/** Set GS primitive type of drawing. */
prim_t prim;

/**
 * Color look up table.
 * Needed for texture.
 */
clutbuffer_t clut;

/**
 * Level of details.
 * Needed for texture.
 */
lod_t lod;

u8 context = 0;
packet2_t* zbyszek_packet;
packet2_t* vif_packets[2] __attribute__((aligned(64)));
packet2_t* curr_vif_packet;
} // namespace

namespace egg::ps2::graphics
{
void draw_mesh(const Matrix& mesh_to_screen_matrix, int num_verts, const Vector* pos, const Vector* colors)
{
	static bool initialized = false;

	if (!initialized)
	{
		// Define the triangle primitive we want to use.
		prim.type         = PRIM_TRIANGLE_STRIP;
		prim.shading      = PRIM_SHADE_GOURAUD;
		prim.mapping      = DRAW_DISABLE;
		prim.fogging      = DRAW_DISABLE;
		prim.blending     = DRAW_DISABLE;
		prim.antialiasing = DRAW_DISABLE;
		prim.mapping_type = PRIM_MAP_ST;
		prim.colorfix     = PRIM_UNFIXED;

		printf("initializing vif packet...\n");
		zbyszek_packet = packet2_create(10, P2_TYPE_NORMAL, P2_MODE_CHAIN, 1);
		vif_packets[0] = packet2_create(1000, P2_TYPE_NORMAL, P2_MODE_CHAIN, 1);
		vif_packets[1] = packet2_create(1000, P2_TYPE_NORMAL, P2_MODE_CHAIN, 1);

		initialized = true;
		printf("Initialized\n");
	}

	packet2_reset(zbyszek_packet, 0);
	packet2_add_float(zbyszek_packet, 2048.0F);                   // scale
	packet2_add_float(zbyszek_packet, 2048.0F);                   // scale
	packet2_add_float(zbyszek_packet, ((float)0xFFFFFF) / 32.0F); // scale
	packet2_add_s32(zbyszek_packet, num_verts);                   // vertex count
	packet2_utils_gs_add_prim_giftag(zbyszek_packet, &prim, num_verts, DRAW_RGBAQ_REGLIST, 2, 0);
	u8 j = 0; // RGBA
	for (j = 0; j < 4; j++)
		packet2_add_u32(zbyszek_packet, 128);

	curr_vif_packet = vif_packets[context];
	packet2_reset(curr_vif_packet, 0);

	packet2_utils_vu_add_unpack_data(curr_vif_packet, 0, (void*)&mesh_to_screen_matrix, 4, 0);

	u32 vif_added_bytes = 0; // zero because now we will use TOP register (double buffer)
	                         // we don't wan't to unpack at 8 + beggining of buffer, but at
	                         // the beggining of the buffer

	// Merge packets
	packet2_utils_vu_add_unpack_data(curr_vif_packet, vif_added_bytes, zbyszek_packet->base, packet2_get_qw_count(zbyszek_packet), 1);
	vif_added_bytes += packet2_get_qw_count(zbyszek_packet);

	packet2_utils_vu_add_unpack_data(curr_vif_packet, vif_added_bytes, (void*)pos, num_verts, 1);
	vif_added_bytes += num_verts;

	packet2_utils_vu_add_start_program(curr_vif_packet, 0);
	packet2_utils_vu_add_end_tag(curr_vif_packet);

	dma_channel_wait(DMA_CHANNEL_VIF1, 0);

	dma_channel_send_packet2(curr_vif_packet, DMA_CHANNEL_VIF1, 1);

	// Switch packet, so we can proceed during DMA transfer
	context = !context;
}
} // namespace egg::ps2::graphics