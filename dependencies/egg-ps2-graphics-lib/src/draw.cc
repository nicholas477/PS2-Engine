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
packet2_t* base_packet[2] __attribute__((aligned(64)));
packet2_t* vif_packets[2] __attribute__((aligned(64)));

packet2_t* curr_vif_packet;
packet2_t* curr_base_packet;

VECTOR* c_verts[2] __attribute__((aligned(128)));
VECTOR* curr_vert_array;

void draw_strip(const Matrix& mesh_to_screen_matrix, int num_verts, const Vector* pos)
{
	static bool initialized = false;

	assert(num_verts > 2);

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

		printf("initializing vif packets...\n");
		base_packet[0] = packet2_create(16, P2_TYPE_NORMAL, P2_MODE_CHAIN, 1);
		base_packet[1] = packet2_create(16, P2_TYPE_NORMAL, P2_MODE_CHAIN, 1);
		vif_packets[0] = packet2_create(4000, P2_TYPE_NORMAL, P2_MODE_CHAIN, 1);
		vif_packets[1] = packet2_create(4000, P2_TYPE_NORMAL, P2_MODE_CHAIN, 1);

		c_verts[0] = (VECTOR*)memalign(128, sizeof(VECTOR) * 128);
		c_verts[1] = (VECTOR*)memalign(128, sizeof(VECTOR) * 128);

		initialized = true;
		printf("Initialized\n");
	}

	curr_base_packet = base_packet[context];
	curr_vif_packet  = vif_packets[context];
	curr_vert_array  = c_verts[context];

	packet2_reset(curr_vif_packet, 0);
	packet2_reset(curr_base_packet, 0);

	memcpy(curr_vert_array, pos, num_verts * sizeof(Vector));

	// 0
	for (int i = 0; i < 4; ++i)
	{
		packet2_add_u128(curr_base_packet, ((u128*)&mesh_to_screen_matrix)[i]);
	}

	// 4
	packet2_add_float(curr_base_packet, 2048.0F);                   // scale
	packet2_add_float(curr_base_packet, 2048.0F);                   // scale
	packet2_add_float(curr_base_packet, ((float)0xFFFFFF) / 32.0F); // scale
	packet2_add_s32(curr_base_packet, num_verts);                   // vert count

	// 5
	packet2_utils_gs_add_prim_giftag(curr_base_packet, &prim, num_verts, DRAW_RGBAQ_REGLIST, 2, 0);

	// 6
	u8 j = 0; // RGBA
	for (j = 0; j < 4; j++)
		packet2_add_u32(curr_base_packet, 128);

	u32 vif_added_qws = 0; // zero because now we will use TOP register (double buffer)
	                       // we don't wan't to unpack at 8 + beggining of buffer, but at
	                       // the beggining of the buffer

	packet2_utils_vu_add_unpack_data(curr_vif_packet, vif_added_qws, curr_base_packet->base, packet2_get_qw_count(curr_base_packet), 0);
	vif_added_qws += packet2_get_qw_count(curr_base_packet);

	packet2_utils_vu_add_unpack_data(curr_vif_packet, vif_added_qws, (void*)curr_vert_array, num_verts, 0);
	vif_added_qws += num_verts;

	packet2_utils_vu_add_start_program(curr_vif_packet, 0);
	packet2_utils_vu_add_end_tag(curr_vif_packet);

	//printf("Waiting for vif1...\n");
	dma_channel_wait(DMA_CHANNEL_VIF1, 0);
	dma_channel_send_packet2(curr_vif_packet, DMA_CHANNEL_VIF1, 1);

	// Switch packet, so we can proceed during DMA transfer
	context ^= 1;
}

} // namespace

namespace egg::ps2::graphics
{
void draw_mesh(const Matrix& mesh_to_screen_matrix, int num_verts, const Vector* pos)
{
	// Draw in blocks of 16 verts each
	for (int32_t i = 0; i <= num_verts; i += 16)
	{
		const auto i_start = std::max(i - 2, (int32_t)0);
		const auto i_end   = std::min((int)i + 16, num_verts);
		assert(i_start != i_end);

		draw_strip(mesh_to_screen_matrix, i_end - i_start, pos + i_start);
	}
}
} // namespace egg::ps2::graphics