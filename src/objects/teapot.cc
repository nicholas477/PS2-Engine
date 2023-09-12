#include "objects/teapot.hpp"
#include "objects/teapot_mesh.inc"
#include "utils/rendering.hpp"
#include "utils/packet.hpp"

#include "renderer/vu1programs/draw_3D.hpp"

#include <malloc.h>
#include <stdio.h>

#include <draw.h>
#include <draw3d.h>

#include <graph.h>
#include <dma_tags.h>
#include <gif_tags.h>
#include <gs_psm.h>

#include <dma.h>

/** Data of our texture (24bit, RGB8) */
extern unsigned char zbyszek[];

static inline bool cullBackFacingTriangle(const Vector* eye, const Vector* v0,
                                          const Vector* v1, const Vector* v2)
{
	// Plane equation of the triangle will give us its orientation
	// which can be compared with the viewer's world position.
	//
	// const Vector d = crossProduct(*v2 - *v0, *v1 - *v0);
	// const Vector c = *v0 - *eye;
	// return dotProduct3(c, d) <= 0.0f;

	const Vector d = (*v2 - *v0).cross(*v1 - *v0);
	//const Vector c = *v0 - *eye; // eye is always 0 in view space
	return v0->dot(d) <= 0.0f; //c.dot(d) <= 0.0f
}

static class teapot_render_proxy
{
public:
	struct alignas(128) aligned_vector: public Vector
	{
		aligned_vector& operator=(const VECTOR& _vector)
		{
			memcpy(vector, _vector, sizeof(VECTOR));
			return *this;
		}
	};

	VECTOR *c_verts __attribute__((aligned(128))), *c_sts __attribute__((aligned(128)));

	teapot_render_proxy()
	{
		c_verts = (VECTOR*)memalign(128, sizeof(VECTOR) * faces_count);
		c_sts   = (VECTOR*)memalign(128, sizeof(VECTOR) * faces_count);

		for (int i = 0; i < faces_count; i++)
		{
			c_verts[i][0] = vertices[faces[i]][0];
			c_verts[i][1] = vertices[faces[i]][1];
			c_verts[i][2] = vertices[faces[i]][2];
			c_verts[i][3] = vertices[faces[i]][3];

			c_sts[i][0] = sts[faces[i]][0];
			c_sts[i][1] = sts[faces[i]][1];
			c_sts[i][2] = sts[faces[i]][2];
			c_sts[i][3] = sts[faces[i]][3];
		}

		// Define the triangle primitive we want to use.
		prim.type         = PRIM_TRIANGLE;
		prim.shading      = PRIM_SHADE_GOURAUD;
		prim.mapping      = DRAW_ENABLE;
		prim.fogging      = DRAW_DISABLE;
		prim.blending     = DRAW_ENABLE;
		prim.antialiasing = DRAW_DISABLE;
		prim.mapping_type = PRIM_MAP_ST;
		prim.colorfix     = PRIM_UNFIXED;
	}

	void on_gs_init()
	{
		texbuff.width   = 128;
		texbuff.psm     = GS_PSM_24;
		texbuff.address = graph_vram_allocate(128, 128, GS_PSM_24, GRAPH_ALIGN_BLOCK);

		printf("Uploading texture.....\n");

		// Upload the texture
		packet2_inline<50> packet(P2_TYPE_NORMAL, P2_MODE_CHAIN, false);
		packet.update(draw_texture_transfer, zbyszek, 128, 128, GS_PSM_24, texbuff.address, texbuff.width);
		packet.update(draw_texture_flush);
		packet.send(DMA_CHANNEL_GIF, true);
		dma_wait_fast();
	}

	void render(const gs::gs_state& gs_state, const transform_component& transform)
	{
		//return;
		//printf("rendering teapot..... context: %d\n", gs_state.get_context());
		Matrix local_world = Matrix::from_location_and_rotation(transform.get_location(), transform.get_rotation());

		// Texture lod
		lod_t lod;
		lod.calculation = LOD_USE_K;
		lod.max_level   = 0;
		lod.mag_filter  = LOD_MAG_NEAREST;
		lod.min_filter  = LOD_MIN_NEAREST;
		lod.l           = 0;
		lod.k           = 0;

		/** 
		 * Color look up table. 
		 * Needed for texture. 
		 */
		clutbuffer_t clut;
		clut.storage_mode = CLUT_STORAGE_MODE1;
		clut.start        = 0;
		clut.psm          = 0;
		clut.load_method  = CLUT_NO_LOAD;
		clut.address      = 0;


		texbuff.info.width      = draw_log2(128);
		texbuff.info.height     = draw_log2(128);
		texbuff.info.components = TEXTURE_COMPONENTS_RGB;
		texbuff.info.function   = TEXTURE_FUNCTION_DECAL;

		// Create the local world-to-screen matrix.
		Matrix local_screen;
		create_local_screen(local_screen, local_world, const_cast<float*>(gs_state.world_view.matrix), const_cast<float*>(gs_state.view_screen.matrix));

		packet2 teapot_draw_packet(10, P2_TYPE_NORMAL, P2_MODE_CHAIN, true);
		//packet2_t* teapot_draw_packet = packet2_create(10, P2_TYPE_NORMAL, P2_MODE_CHAIN, 1);
		packet2_add_float(teapot_draw_packet, 2048.0F);        // scale
		packet2_add_float(teapot_draw_packet, 2048.0F);        // scale
		packet2_add_float(teapot_draw_packet, 32.0F);          // scale
		packet2_add_s32(teapot_draw_packet, (u32)faces_count); // vertex count
		packet2_utils_gif_add_set(teapot_draw_packet, 1);
		packet2_utils_gs_add_lod(teapot_draw_packet, &lod);
		packet2_utils_gs_add_texbuff_clut(teapot_draw_packet, &texbuff, &clut);
		packet2_utils_gs_add_prim_giftag(teapot_draw_packet, &prim, faces_count, DRAW_STQ2_REGLIST, 3, 0);
		u8 j = 0; // RGBA
		for (j = 0; j < 4; j++)
			packet2_add_u32(teapot_draw_packet, 128);

		auto& curr_vif_packet = gs_state.get_current_packet();
		curr_vif_packet.reset(false);

		//Add matrix at the beggining of VU mem (skip TOP)
		packet2_utils_vu_add_unpack_data(curr_vif_packet, 0, local_screen.matrix, 8, 0);

		u32 vif_added_bytes = 0; // zero because now we will use TOP register (double buffer)
		                         // we don't wan't to unpack at 8 + beggining of buffer, but at
		                         // the beggining of the buffer

		check(((uintptr_t)teapot_draw_packet->base % 64) == 0);

		// Merge packets
		packet2_utils_vu_add_unpack_data(curr_vif_packet, vif_added_bytes, teapot_draw_packet->base, packet2_get_qw_count(teapot_draw_packet), 1);
		vif_added_bytes += packet2_get_qw_count(teapot_draw_packet);

		// Add vertices
		packet2_utils_vu_add_unpack_data(curr_vif_packet, vif_added_bytes, c_verts, faces_count, 1);
		vif_added_bytes += faces_count; // one VECTOR is size of qword

		// Add sts
		packet2_utils_vu_add_unpack_data(curr_vif_packet, vif_added_bytes, c_sts, faces_count, 1);
		vif_added_bytes += faces_count;

		packet2_utils_vu_add_start_program(curr_vif_packet, draw_3D::get().getDestinationAddress());
		packet2_utils_vu_add_end_tag(curr_vif_packet);


		dma_channel_wait(DMA_CHANNEL_VIF1, 0);
		curr_vif_packet.send(DMA_CHANNEL_VIF1, true);
		dma_channel_wait(DMA_CHANNEL_VIF1, 0);

		gs_state.flip_context();
	}

	AABB get_bounds() const
	{
		// Cache the bounds
		static AABB out = compute_bounds();
		return out;
	}

protected:
	AABB compute_bounds() const
	{
		AABB out;
		out.Min = vertices[0];
		out.Max = vertices[0];
		for (int i = 0; i < vertex_count; ++i)
		{
			out.Min.x = std::min(out.Min.x, vertices[i][0]);
			out.Min.y = std::min(out.Min.y, vertices[i][1]);
			out.Min.z = std::min(out.Min.z, vertices[i][2]);

			out.Max.x = std::max(out.Max.x, vertices[i][0]);
			out.Max.y = std::max(out.Max.y, vertices[i][1]);
			out.Max.z = std::max(out.Max.z, vertices[i][2]);
		}

		return out;
	}

private:
	prim_t prim;

	texbuffer_t texbuff;
} _teapot_render_proxy;

static class teapot_render_proxy_initializer: public renderable
{
public:
	teapot_render_proxy_initializer()
	    : renderable(true)
	{
	}

	virtual void on_gs_init() override
	{
		_teapot_render_proxy.on_gs_init();
	}

} _teapot_render_proxy_initializer;

teapot::teapot()
    : collision(&transform)
{
	render_proxy = &_teapot_render_proxy;

	collision.set_local_bounds(_teapot_render_proxy.get_bounds());
}

void teapot::render(const gs::gs_state& gs_state)
{
	render_proxy->render(gs_state, transform);

	color_t color;
	color.r = 32;
	color.g = 32;
	color.b = 32;
	color.a = 128.f;
	color.q = 1.f;

	collision.render(gs_state, color);
}