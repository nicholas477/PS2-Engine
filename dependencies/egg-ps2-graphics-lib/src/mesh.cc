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

void draw_strip(const Matrix& mesh_to_screen_matrix, const mesh_descriptor& mesh)
{
	assert(mesh.is_valid());

	// Define the triangle primitive we want to use.
	prim.type         = PRIM_TRIANGLE_STRIP;
	prim.shading      = PRIM_SHADE_GOURAUD;
	prim.mapping      = DRAW_DISABLE;
	prim.fogging      = DRAW_DISABLE;
	prim.blending     = DRAW_DISABLE;
	prim.antialiasing = DRAW_DISABLE;
	prim.mapping_type = PRIM_MAP_ST;
	prim.colorfix     = PRIM_UNFIXED;

	packet2_reset(get_current_vif_packet(), 0);

	packet2_utils_vu_open_unpack(get_current_vif_packet(), 0, 1);
	{
		// 0
		for (int i = 0; i < 4; ++i)
		{
			packet2_add_u128(get_current_vif_packet(), ((u128*)&mesh_to_screen_matrix)[i]);
		}

		// 4
		packet2_add_float(get_current_vif_packet(), 2048.0F);                    // scale
		packet2_add_float(get_current_vif_packet(), 2048.0F);                    // scale
		packet2_add_float(get_current_vif_packet(), ((float)0xFFFFFF) / -32.0F); // scale
		packet2_add_u32(get_current_vif_packet(), mesh.num_verts);               // vert count

		// 5
		packet2_utils_gs_add_prim_giftag(get_current_vif_packet(), &prim, mesh.num_verts, DRAW_RGBAQ_REGLIST, 2, 0);

		// 6
		u8 j = 0; // RGBA
		for (j = 0; j < 4; j++)
			packet2_add_u32(get_current_vif_packet(), 128);
	}
	packet2_utils_vu_close_unpack(get_current_vif_packet());

	// Position data
	packet2_utils_vu_add_unpack_data(get_current_vif_packet(), 8, (void*)mesh.pos, mesh.num_verts, 1);

	// Color data
	packet2_utils_vu_add_unpack_data(get_current_vif_packet(), 8 + mesh.num_verts, (void*)mesh.color, mesh.num_verts, 1);

	//assert((8 + (mesh.num_verts * 4)) < 496);

	packet2_utils_vu_add_start_program(get_current_vif_packet(), mesh.vu_program_addr);
	packet2_utils_vu_add_end_tag(get_current_vif_packet());

	dma_channel_wait(DMA_CHANNEL_VIF1, 0);
	dma_channel_send_packet2(get_current_vif_packet(), DMA_CHANNEL_VIF1, 1);
	dma_channel_wait(DMA_CHANNEL_VIF1, 0);

	flip_vip_packet_context();
}

} // namespace

namespace egg::ps2::graphics
{
void draw_mesh(const Matrix& mesh_to_screen_matrix, const mesh_descriptor& mesh)
{
	assert(mesh.is_valid());

	// TODO: calculate this dynamically based on how much stuff is being put into vu mem
	static constexpr s32 verts_per_call = 96;

	for (u32 i = 0;;)
	{
		const u32 i_end     = std::min(i + verts_per_call, mesh.num_verts);
		const u32 num_verts = i_end - i;
		assert(i != i_end);
		assert(num_verts >= 3);
		assert(num_verts <= verts_per_call);

		{
			mesh_descriptor strip = mesh;
			strip.pos             = strip.pos + i;
			if (strip.color != nullptr)
			{
				strip.color = strip.color + i;
			}

			strip.num_verts = num_verts;

			draw_strip(mesh_to_screen_matrix, strip);
		}

		if (i_end >= mesh.num_verts)
		{
			break;
		}
		else
		{
			// For the next batch, we also have to draw the 2 verts from the
			// previous patch so that we don't miss drawing any triangles
			i = i_end - 2;
		}
	}
}

bool mesh_descriptor::is_valid(bool print_why_invalid) const
{
	bool all_valid = true;
	if ((__uintptr_t)pos == 0)
	{
		if (print_why_invalid)
		{
			printf("Mesh invalid! Pos pointer is nullptr!\n");
		}
		all_valid &= false;
	}

	if (!__is_aligned(pos, 16))
	{
		if (print_why_invalid)
		{
			printf("Mesh invalid! Pos pointer not aligned to a 16 byte address!\n");
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