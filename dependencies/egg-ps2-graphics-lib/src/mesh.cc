#include "egg-ps2-graphics-lib/mesh.hpp"
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

using namespace egg::ps2::graphics;

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
packet2_t* vif_packets[2] __attribute__((aligned(64)));

packet2_t* curr_vif_packet;

void draw_strip(const Matrix& mesh_to_screen_matrix, const mesh_descriptor& mesh)
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

		printf("initializing vif packets...\n");
		vif_packets[0] = packet2_create(4000, P2_TYPE_NORMAL, P2_MODE_CHAIN, 1);
		vif_packets[1] = packet2_create(4000, P2_TYPE_NORMAL, P2_MODE_CHAIN, 1);

		initialized = true;
		printf("Initialized\n");
	}

	curr_vif_packet = vif_packets[context];

	packet2_reset(curr_vif_packet, 0);

	packet2_utils_vu_open_unpack(curr_vif_packet, 0, false);
	{
		// 0
		for (int i = 0; i < 4; ++i)
		{
			packet2_add_u128(curr_vif_packet, ((u128*)&mesh_to_screen_matrix)[i]);
		}

		// 4
		packet2_add_float(curr_vif_packet, 2048.0F);                   // scale
		packet2_add_float(curr_vif_packet, 2048.0F);                   // scale
		packet2_add_float(curr_vif_packet, ((float)0xFFFFFF) / 32.0F); // scale
		packet2_add_s32(curr_vif_packet, mesh.num_verts);              // vert count

		// 5
		{
			packet2_add_2x_s64(
			    curr_vif_packet,
			    VU_GS_GIFTAG(
			        mesh.num_verts, // Information for GS. Amount of loops
			        1,
			        1,
			        VU_GS_PRIM(
			            prim.type,
			            prim.shading,
			            prim.mapping,
			            prim.fogging,
			            prim.blending,
			            prim.antialiasing,
			            prim.mapping_type,
			            0, // context
			            prim.colorfix),
			        0,
			        2),
			    DRAW_RGBAQ_REGLIST);
		}

		// 6
		u8 j = 0; // RGBA
		for (j = 0; j < 4; j++)
			packet2_add_u32(curr_vif_packet, 128);
	}
	packet2_utils_vu_close_unpack(curr_vif_packet);

	packet2_utils_vu_add_unpack_data(curr_vif_packet, 7, (void*)mesh.pos, mesh.num_verts, 0);

	if (mesh.color != nullptr)
	{
		packet2_utils_vu_add_unpack_data(curr_vif_packet, 7 + mesh.num_verts, (void*)mesh.color, mesh.num_verts, 0);
	}

	packet2_utils_vu_add_start_program(curr_vif_packet, 0);
	packet2_utils_vu_add_end_tag(curr_vif_packet);
	dma_channel_wait(DMA_CHANNEL_VIF1, 0);
	dma_channel_send_packet2(curr_vif_packet, DMA_CHANNEL_VIF1, 1);
}

} // namespace

// TODO: calculate this dynamically based on how much stuff is being put into vu mem
static constexpr s32 verts_per_call = 128;

namespace egg::ps2::graphics
{
void draw_mesh(const Matrix& mesh_to_screen_matrix, const mesh_descriptor& mesh)
{
	assert(mesh.is_valid());

	// Draw in blocks of 16 verts each
	for (u32 i = 0; i <= mesh.num_verts; i += verts_per_call)
	{
		const u32 i_start   = (u32)std::max((s32)i - 2, 0);
		const u32 i_end     = std::min(i + verts_per_call, mesh.num_verts);
		const u32 num_verts = i_end - i_start;
		assert(i_start != i_end);
		assert(num_verts >= 3);

		mesh_descriptor strip = mesh;
		strip.pos             = strip.pos + i_start;
		if (strip.color != nullptr)
		{
			strip.color = strip.color + i_start;
		}

		strip.num_verts = num_verts;

		draw_strip(mesh_to_screen_matrix, strip);
	}
}

bool mesh_descriptor::is_valid(bool print_why_invalid) const
{
	bool all_valid = true;
	if (!__is_aligned(pos, 16))
	{
		if (print_why_invalid)
		{
			printf("Mesh invalid! Pos pointer not aligned to a 16 byte address!\n");
		}
		all_valid &= false;
	}

	if ((__uintptr_t)pos == 0)
	{
		if (print_why_invalid)
		{
			printf("Mesh invalid! Pos pointer is nullptr!\n");
		}
		all_valid &= false;
	}

	if (!__is_aligned(color, 16))
	{
		if (print_why_invalid)
		{
			printf("Mesh invalid! Color pointer not aligned to a 16 byte address!\n");
		}
		all_valid &= false;
	}

	if (num_verts < 3)
	{
		if (print_why_invalid)
		{
			printf("Mesh invalid! num_verts less than 3! Actual value: %u\n", num_verts);
		}
		all_valid &= false;
	}

	if (vu_program_addr > 1023)
	{
		if (print_why_invalid)
		{
			printf("Mesh invalid! vu_program_addr greater than 1023! Actual value: %u\n", vu_program_addr);
		}
		all_valid &= false;
	}

	return all_valid;
}
} // namespace egg::ps2::graphics