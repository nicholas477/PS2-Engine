#include "objects/teapot.hpp"
#include "objects/teapot_mesh.inc"

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
	const Vector d = (*v2 - *v0).cross((*v1 - *v0));
	const Vector c = *v0 - *eye;
	return c.dot(d) >= 0.0f;
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

	[[nodiscard]] qword_t* render(qword_t* q, const gs::gs_state& gs_state, const transform_component& transform)
	{
		Matrix local_world = Matrix::from_location_and_rotation(transform.get_location(), transform.get_rotation());
		Matrix local_light = transform.get_rotation().to_rotation_matrix();

		MATRIX local_screen;

		// Now grab our qword pointer and increment past the dmatag.
		qword_t* dmatag = q;
		q++;

		// Create the local_screen matrix.
		create_local_screen(local_screen, local_world.matrix, const_cast<float*>(gs_state.view_world.matrix), const_cast<float*>(gs_state.view_screen.matrix));

		// Calculate the normal values.
		calculate_normals(temp_normals, vertex_count, normals, local_light.matrix);

		// Calculate the lighting values.
		calculate_lights(temp_lights, vertex_count, temp_normals, (VECTOR*)gs_state.lights.directions.data(),
		                 (VECTOR*)gs_state.lights.colors.data(), (int*)gs_state.lights.types.data(), gs_state.lights.count());

		// Calculate the colour values after lighting.
		calculate_colours(temp_colours, vertex_count, colours, temp_lights);

		// Calculate the vertex values.
		calculate_vertices(temp_vertices, vertex_count, vertices, local_screen);

		// Convert floating point vertices to fixed point and translate to center of
		// screen.
		draw_convert_xyz(xyz, 2048, 2048, 32, vertex_count,
		                 (vertex_f_t*)temp_vertices);

		// Convert floating point colours to fixed point.
		draw_convert_rgbq(rgbaq, vertex_count, (vertex_f_t*)temp_vertices,
		                  (color_f_t*)temp_colours, color.a);

		// Draw the triangles using triangle primitive type.
		q = draw_prim_start(q, 0, &prim, &color);

		//Vector eye_pos = gs_state.view_world.transform_vector(Vector());

		for (int i = 0; i < points_count / 3; ++i)
		{
			const Vector* v0 = (Vector*)&temp_vertices[points[(i * 3)]];
			const Vector* v1 = (Vector*)&temp_vertices[points[(i * 3) + 1]];
			const Vector* v2 = (Vector*)&temp_vertices[points[(i * 3) + 2]];

			// if (cullBackFacingTriangle(&eye_pos, v0, v1, v2))
			// {
			// 	continue;
			// }


			q->dw[0] = rgbaq[points[(i * 3)]].rgbaq;
			q->dw[1] = xyz[points[(i * 3)]].xyz;
			q++;

			q->dw[0] = rgbaq[points[(i * 3) + 1]].rgbaq;
			q->dw[1] = xyz[points[(i * 3) + 1]].xyz;
			q++;


			q->dw[0] = rgbaq[points[(i * 3) + 2]].rgbaq;
			q->dw[1] = xyz[points[(i * 3) + 2]].xyz;
			q++;
		}

		q = draw_prim_end(q, 2, DRAW_RGBAQ_REGLIST);

		// Define our dmatag for the dma chain.
		DMATAG_CNT(dmatag, q - dmatag - 1, 0, 0, 0);

		return q;
	}

	AABB get_bounds() const
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

qword_t*
teapot::render(qword_t* q, const gs::gs_state& gs_state)
{
	q = render_proxy->render(q, gs_state, transform);

	color_t color;
	color.r = 32;
	color.g = 32;
	color.b = 32;
	color.a = 128.f;
	color.q = 1.f;

	q = collision.render(q, gs_state, color);
	return q;
}