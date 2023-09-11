#include "objects/teapot.hpp"
#include "objects/teapot_mesh.inc"
#include "utils/rendering.hpp"
#include "utils/packet.hpp"

#include "renderer/vu1programs/draw_3D.hpp"

#include <malloc.h>
#include <stdio.h>

#include <draw.h>
#include <draw3d.h>

#include <dma_tags.h>
#include <gif_tags.h>
#include <gs_psm.h>

#include <dma.h>

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
	teapot_render_proxy()
	{
		temp_normals  = (float(*)[4])memalign(128, sizeof(VECTOR) * vertex_count);
		temp_lights   = (float(*)[4])memalign(128, sizeof(VECTOR) * vertex_count);
		temp_colours  = (float(*)[4])memalign(128, sizeof(VECTOR) * vertex_count);
		temp_vertices = (float(*)[4])memalign(128, sizeof(VECTOR) * vertex_count);

		xyz   = (xyz_t*)memalign(128, sizeof(u64) * vertex_count);
		rgbaq = (color_t*)memalign(128, sizeof(u64) * vertex_count);

		// Define the triangle primitive we want to use.
		prim.type         = PRIM_TRIANGLE;
		prim.shading      = PRIM_SHADE_GOURAUD;
		prim.mapping      = DRAW_DISABLE;
		prim.fogging      = DRAW_DISABLE;
		prim.blending     = DRAW_ENABLE;
		prim.antialiasing = DRAW_DISABLE;
		prim.mapping_type = DRAW_DISABLE;
		prim.colorfix     = PRIM_UNFIXED;

		color.r = 0x80;
		color.g = 0x80;
		color.b = 0x80;
		color.a = 0x80;
		color.q = 1.0f;
	}

	void render(const gs::gs_state& gs_state, const transform_component& transform)
	{
		Matrix local_world = Matrix::from_location_and_rotation(transform.get_location(), transform.get_rotation());
		//Matrix local_light = transform.get_rotation().to_rotation_matrix();

		MATRIX local_screen;

		// Now grab our qword pointer and increment past the dmatag.
		// qword_t* dmatag = q;
		// q++;

		//texbuffer_t texbuff;

		// Texture lod
		// lod_t lod;
		// lod.calculation = LOD_USE_K;
		// lod.max_level   = 0;
		// lod.mag_filter  = LOD_MAG_NEAREST;
		// lod.min_filter  = LOD_MIN_NEAREST;
		// lod.l           = 0;
		// lod.k           = 0;

		/** 
		 * Color look up table. 
		 * Needed for texture. 
		 */
		// clutbuffer_t clut;
		// clut.storage_mode = CLUT_STORAGE_MODE1;
		// clut.start        = 0;
		// clut.psm          = 0;
		// clut.load_method  = CLUT_NO_LOAD;
		// clut.address      = 0;

		// Create the local world-to-screen matrix.
		create_local_screen(local_screen, local_world.matrix, const_cast<float*>(gs_state.world_view.matrix), const_cast<float*>(gs_state.view_screen.matrix));

		{
			// printf("making new packet...........!!!!!!!!!!!!!!!!!\n");
			// packet2 teapot_draw_packet = packet2(10, P2_TYPE_NORMAL, P2_MODE_CHAIN, true);
			// printf("making new packet...........!!!!!!!!!!!!!!!!! 1\n");
			// packet2_add_float(teapot_draw_packet, 2048.0F); // scale
			// printf("making new packet...........!!!!!!!!!!!!!!!!! 2\n");
			// packet2_add_float(teapot_draw_packet, 2048.0F); // scale
			// printf("making new packet...........!!!!!!!!!!!!!!!!! 3\n");
			// packet2_add_float(teapot_draw_packet, ((float)0xFFFFFF) / 32.0F); // scale
			// printf("making new packet...........!!!!!!!!!!!!!!!!! 4\n");
			// packet2_add_s32(teapot_draw_packet, vertex_count); // vertex count
			// printf("making new packet...........!!!!!!!!!!!!!!!!! 5\n");
			// packet2_utils_gif_add_set(teapot_draw_packet, 1);
			// printf("making new packet...........!!!!!!!!!!!!!!!!! 6\n");
			// packet2_utils_gs_add_lod(teapot_draw_packet, &lod);
			// printf("making new packet...........!!!!!!!!!!!!!!!!! 7\n");
			// packet2_utils_gs_add_texbuff_clut(teapot_draw_packet, &texbuff, &clut);
			// printf("making new packet...........!!!!!!!!!!!!!!!!! 8\n");
			// packet2_utils_gs_add_prim_giftag(teapot_draw_packet, &prim, vertex_count, DRAW_STQ2_REGLIST, 3, 0);
			// printf("making new packet...........!!!!!!!!!!!!!!!!! 9\n");
			// u8 j = 0; // RGBA
			// for (j = 0; j < 4; j++)
			// 	packet2_add_u32(teapot_draw_packet, 128);

			// printf("making new packet...........!!!!!!!!!!!!!!!!! 10\n");
			// packet2_reset(teapot_draw_packet, 0);
			// printf("making new packet...........!!!!!!!!!!!!!!!!! 11\n");

			// // Add matrix at the beggining of VU mem (skip TOP)
			// packet2_utils_vu_add_unpack_data(teapot_draw_packet, 0, &local_screen, 8, 0);
			// printf("making new packet...........!!!!!!!!!!!!!!!!! 12\n");

			// u32 vif_added_bytes = 0; // zero because now we will use TOP register (double buffer)
			//                          // we don't wan't to unpack at 8 + beggining of buffer, but at
			//                          // the beggining of the buffer

			// // Merge packets
			// packet2_utils_vu_add_unpack_data(curr_vif_packet, vif_added_bytes, teapot_draw_packet->base, packet2_get_qw_count(teapot_draw_packet), 1);
			// //vif_added_bytes += packet2_get_qw_count(zbyszek_packet);

			// printf("making new packet...........!!!!!!!!!!!!!!!!! 13\n");
			// // // Add vertices
			// packet2_utils_vu_add_unpack_data(teapot_draw_packet, vif_added_bytes, temp_vertices, vertex_count, 1);
			// vif_added_bytes += vertex_count; // one VECTOR is size of qword

			// printf("making new packet...........!!!!!!!!!!!!!!!!! 14\n");
			// // // Add sts
			// packet2_utils_vu_add_unpack_data(teapot_draw_packet, vif_added_bytes, temp_vertices, vertex_count, 1);
			// vif_added_bytes += vertex_count;

			// printf("making new packet...........!!!!!!!!!!!!!!!!! 15\n");
			// packet2_utils_vu_add_start_program(teapot_draw_packet, draw_3D::get().getDestinationAddress());
			// packet2_utils_vu_add_end_tag(teapot_draw_packet);
			// printf("sending packet...........!!!!!!!!!!!!!!!!!\n");
			// dma_channel_wait(DMA_CHANNEL_VIF1, 0);
			// dma_channel_send_packet2(teapot_draw_packet, DMA_CHANNEL_VIF1, 1);
		}

		// Calculate the normal values.
		// calculate_normals(temp_normals, vertex_count, normals, local_light.matrix);

		// // Calculate the lighting values.
		// calculate_lights(temp_lights, vertex_count, temp_normals, (VECTOR*)gs_state.lights.directions.data(),
		//                  (VECTOR*)gs_state.lights.colors.data(), (int*)gs_state.lights.types.data(), gs_state.lights.count());

		// // Calculate the colour values after lighting.
		// calculate_colours(temp_colours, vertex_count, colours, temp_lights);

		// // Calculate the vertex values.
		// calculate_vertices(temp_vertices, vertex_count, vertices, local_screen);

		// // Convert floating point vertices to fixed point and translate to center of
		// // screen.
		// draw_convert_xyz(xyz, 2048, 2048, 32, vertex_count,
		//                  (vertex_f_t*)temp_vertices);

		// // Convert floating point colours to fixed point.
		// draw_convert_rgbq(rgbaq, vertex_count, (vertex_f_t*)temp_vertices,
		//                   (color_f_t*)temp_colours, color.a);

		// // Draw the triangles using triangle primitive type.
		// q = draw_prim_start(q, 0, &prim, &color);

		// for (int i = 0; i < points_count / 3; ++i)
		// {
		// 	const Vector* v0 = (Vector*)&temp_vertices[points[(i * 3)]];
		// 	const Vector* v1 = (Vector*)&temp_vertices[points[(i * 3) + 1]];
		// 	const Vector* v2 = (Vector*)&temp_vertices[points[(i * 3) + 2]];

		// 	if (cullBackFacingTriangle(&Vector::zero, v0, v1, v2))
		// 	{
		// 		continue;
		// 	}

		// 	q->dw[0] = rgbaq[points[(i * 3)]].rgbaq;
		// 	q->dw[1] = xyz[points[(i * 3)]].xyz;
		// 	q++;

		// 	q->dw[0] = rgbaq[points[(i * 3) + 1]].rgbaq;
		// 	q->dw[1] = xyz[points[(i * 3) + 1]].xyz;
		// 	q++;


		// 	q->dw[0] = rgbaq[points[(i * 3) + 2]].rgbaq;
		// 	q->dw[1] = xyz[points[(i * 3) + 2]].xyz;
		// 	q++;
		// }

		// q = draw_prim_end(q, 2, DRAW_RGBAQ_REGLIST);

		// Define our dmatag for the dma chain.
		// DMATAG_CNT(dmatag, q - dmatag - 1, 0, 0, 0);

		//return q;
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
	xyz_t* xyz;
	color_t* rgbaq;
	prim_t prim;
	color_t color;

	VECTOR* temp_normals;
	VECTOR* temp_lights;
	VECTOR* temp_colours;
	VECTOR* temp_vertices;
} _teapot_render_proxy;

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