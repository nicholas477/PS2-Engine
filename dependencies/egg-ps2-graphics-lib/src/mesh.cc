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
	prim.shading      = PRIM_SHADE_FLAT;
	prim.mapping      = mesh.enable_texture_mapping ? DRAW_ENABLE : DRAW_DISABLE;
	prim.fogging      = mesh.enable_fog ? DRAW_ENABLE : DRAW_DISABLE;
	prim.blending     = DRAW_DISABLE;
	prim.antialiasing = DRAW_DISABLE;
	prim.mapping_type = PRIM_MAP_ST;
	prim.colorfix     = PRIM_UNFIXED;

	packet2_utils_vu_open_unpack(get_current_vif_packet(), 0, 1);
	{
		// 0
		for (int i = 0; i < 4; ++i)
		{
			packet2_add_u128(get_current_vif_packet(), ((u128*)&mesh_to_screen_matrix)[i]);
		}

		// 4
		packet2_add_float(get_current_vif_packet(), mesh.screen_scale.x); // scale
		packet2_add_float(get_current_vif_packet(), mesh.screen_scale.y); // scale
		packet2_add_float(get_current_vif_packet(), mesh.screen_scale.z); // scale
		packet2_add_u32(get_current_vif_packet(), mesh.num_verts);        // vert count

		if (mesh.enable_fog)
		{
			// 5
			// The F in XYZF2 stands for fog
			packet2_utils_gs_add_prim_giftag(get_current_vif_packet(), &prim, mesh.num_verts,
			                                 ((u64)GIF_REG_ST) << 0 | ((u64)GIF_REG_RGBAQ) << 4 | ((u64)GIF_REG_XYZF2) << 8,
			                                 3, 0);
		}
		else
		{
			// 5
			packet2_utils_gs_add_prim_giftag(get_current_vif_packet(), &prim, mesh.num_verts,
			                                 DRAW_STQ2_REGLIST,
			                                 3, 0);
		}

		// 6
		u8 j = 0; // RGBA
		for (j = 0; j < 4; j++)
			packet2_add_u32(get_current_vif_packet(), 128);

		// 7
		packet2_add_float(get_current_vif_packet(), mesh.fog_offset); // Offset
		packet2_add_float(get_current_vif_packet(), mesh.fog_scale);  // Scale
		packet2_add_float(get_current_vif_packet(), 0.f);             // padding
		packet2_add_float(get_current_vif_packet(), 0.f);             // padding
	}
	packet2_utils_vu_close_unpack(get_current_vif_packet());

	// Position data
	packet2_utils_vu_add_unpack_data(get_current_vif_packet(), 8, mesh.pos, mesh.num_verts, 1);

	if (mesh.color)
	{
		// Color data
		packet2_utils_vu_add_unpack_data(get_current_vif_packet(), 8 + mesh.num_verts, mesh.color, mesh.num_verts, 1);
	}

	if (mesh.uvs)
	{
		// Color data
		packet2_utils_vu_add_unpack_data(get_current_vif_packet(), 8 + (mesh.num_verts * 2), mesh.uvs, mesh.num_verts, 1);
	}

	assert((8 + (mesh.num_verts * 6)) < 496);

	packet2_utils_vu_add_start_program(get_current_vif_packet(), mesh.vu_program_addr);
}

} // namespace

namespace egg::ps2::graphics
{
mesh_descriptor::mesh_descriptor()
{
	pos   = nullptr;
	color = nullptr;

	// Has to be at least 3
	num_verts = 0;

	enable_texture_mapping = false;

	// Address of the VU program loaded in memory used to perform vertex
	// processing on this mesh.
	//
	// Note: this is a vu1 memory address! Valid values for a vu1 memory address
	// are 0-1023
	vu_program_addr = 0;

	screen_scale = Vector(2048.f, 2048.f, ((float)0xFFFFFF) / 32.0F);

	// Fog settings
	enable_fog = false;

	// The start offset of the fog
	fog_offset = 100.f;

	// fog distance scaling
	fog_scale = -256.f / 1024.f;
}

void mesh_descriptor::set_fog_start_and_end(float fog_start, float fog_end)
{
	fog_offset = fog_start;
	fog_scale  = -256.f / (fog_end - fog_start);
}

void draw_mesh_strip(const Matrix& mesh_to_screen_matrix, const mesh_descriptor& mesh)
{
	assert(mesh.is_valid());

	// TODO: calculate this dynamically based on how much stuff is being put into vu mem
	static constexpr s32 verts_per_call = 80;

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
			if (strip.uvs != nullptr)
			{
				strip.uvs = strip.uvs + i;
			}
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

	if (!__is_aligned(uvs, 16))
	{
		if (print_why_invalid)
		{
			printf("Mesh invalid! UVs pointer not aligned to a 16 byte address!\n");
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