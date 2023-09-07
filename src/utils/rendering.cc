#include "utils/rendering.hpp"

#include "draw.h"
#include "draw2d.h"
#include "draw3d.h"

#include <dma_tags.h>
#include <gif_tags.h>
#include <gs_psm.h>

#include <malloc.h>
#include <stdio.h>

static class aabb_render_proxy
{
public:
	aabb_render_proxy()
	{
		temp_vertices = (float(*)[4])memalign(128, sizeof(VECTOR) * vertex_count);
		xyz           = (xyz_t*)memalign(128, sizeof(u64) * vertex_count);

		// Define the triangle primitive we want to use.
		prim.type         = PRIM_LINE;
		prim.shading      = PRIM_SHADE_FLAT;
		prim.mapping      = DRAW_DISABLE;
		prim.fogging      = DRAW_DISABLE;
		prim.blending     = DRAW_DISABLE;
		prim.antialiasing = DRAW_DISABLE;
		prim.mapping_type = PRIM_MAP_ST;
		prim.colorfix     = PRIM_UNFIXED;
	}

	qword_t* render(qword_t* q, const gs::gs_state& gs_state, const AABB& aabb, color_t color, bool on_top)
	{
		VECTOR vertices[8] = {
		    {aabb.Min.x, aabb.Min.y, aabb.Min.z, 1.000f}, // 0 {-x, -y, -z}
		    {aabb.Max.x, aabb.Min.y, aabb.Min.z, 1.000f}, // 1 { x, -y, -z}
		    {aabb.Max.x, aabb.Max.y, aabb.Min.z, 1.000f}, // 2 { x,  y, -z}
		    {aabb.Max.x, aabb.Max.y, aabb.Max.z, 1.000f}, // 3 { x,  y,  z}

		    {aabb.Min.x, aabb.Max.y, aabb.Max.z, 1.000f}, // 4 {-x,  y,  z}
		    {aabb.Min.x, aabb.Min.y, aabb.Max.z, 1.000f}, // 5 {-x, -y,  z}

		    {aabb.Min.x, aabb.Max.y, aabb.Min.z, 1.000f}, // 6 {-x,  y, -z}
		    {aabb.Max.x, aabb.Min.y, aabb.Max.z, 1.000f}, // 7 { x, -y,  z}
		};

		Matrix local_world = Matrix::UnitMatrix();

		MATRIX local_screen;

		// Now grab our qword pointer and increment past the dmatag.
		qword_t* dmatag = q;
		q++;

		// Create the local_screen matrix.
		create_local_screen(local_screen, local_world.matrix, const_cast<float*>(gs_state.view_world.matrix), const_cast<float*>(gs_state.view_screen.matrix));

		// Calculate the vertex values.
		calculate_vertices(temp_vertices, vertex_count, vertices, local_screen);

		// Convert floating point vertices to fixed point and translate to center of
		// screen.
		draw_convert_xyz(xyz, 2048, 2048, 32, vertex_count,
		                 (vertex_f_t*)temp_vertices);

		// // Draw the triangles using triangle primitive type.
		q = draw_prim_start(q, 0, &prim, &color);

		if (on_top)
		{
			for (int i = 0; i < vertex_count; ++i)
			{
				xyz[i].z = 0xffffffff;
			}
		}

		// Bottom rectangle (-y)
		{
			q->dw[0] = xyz[0].xyz;
			q->dw[1] = xyz[1].xyz;
			q++;

			q->dw[0] = xyz[0].xyz;
			q->dw[1] = xyz[5].xyz;
			q++;

			q->dw[0] = xyz[7].xyz;
			q->dw[1] = xyz[1].xyz;
			q++;

			q->dw[0] = xyz[5].xyz;
			q->dw[1] = xyz[7].xyz;
			q++;
		}

		// Top rectangle (+y)
		{
			q->dw[0] = xyz[6].xyz;
			q->dw[1] = xyz[2].xyz;
			q++;

			q->dw[0] = xyz[6].xyz;
			q->dw[1] = xyz[4].xyz;
			q++;

			q->dw[0] = xyz[3].xyz;
			q->dw[1] = xyz[2].xyz;
			q++;

			q->dw[0] = xyz[4].xyz;
			q->dw[1] = xyz[3].xyz;
			q++;
		}

		// Top/bottom edges
		{
			q->dw[0] = xyz[0].xyz;
			q->dw[1] = xyz[6].xyz;
			q++;

			q->dw[0] = xyz[4].xyz;
			q->dw[1] = xyz[5].xyz;
			q++;

			q->dw[0] = xyz[7].xyz;
			q->dw[1] = xyz[3].xyz;
			q++;

			q->dw[0] = xyz[1].xyz;
			q->dw[1] = xyz[2].xyz;
			q++;
		}

		q = draw_prim_end(q, 2, DRAW_XYZ_REGLIST);

		// Define our dmatag for the dma chain.
		DMATAG_CNT(dmatag, q - dmatag - 1, 0, 0, 0);

		return q;
	}

private:
	static constexpr int vertex_count = 8;
	xyz_t* xyz;
	VECTOR* temp_vertices;

	prim_t prim;
} _aabb_render_proxy;

qword_t* draw_aabb(qword_t* q, const gs::gs_state& gs_state, const AABB& aabb, color_t color, bool on_top)
{
	return _aabb_render_proxy.render(q, gs_state, aabb, color, on_top);
}

void draw_aabb_one_frame(const AABB& aabb, color_t color, bool on_top)
{
	gs::add_renderable_one_frame([=](qword_t* q, const gs::gs_state& state) -> qword_t* {
		return draw_aabb(q, state, aabb, color, on_top);
	});
}

static class line_render_proxy
{
public:
	line_render_proxy()
	{
		temp_vertices = (float(*)[4])memalign(128, sizeof(VECTOR) * vertex_count);
		xyz           = (xyz_t*)memalign(128, sizeof(u64) * vertex_count);

		// Define the triangle primitive we want to use.
		prim.type         = PRIM_LINE;
		prim.shading      = PRIM_SHADE_FLAT;
		prim.mapping      = DRAW_DISABLE;
		prim.fogging      = DRAW_DISABLE;
		prim.blending     = DRAW_DISABLE;
		prim.antialiasing = DRAW_DISABLE;
		prim.mapping_type = PRIM_MAP_ST;
		prim.colorfix     = PRIM_UNFIXED;
	}

	qword_t* render(qword_t* q, const gs::gs_state& gs_state, const Vector& line_start, const Vector& line_end, color_t color, bool on_top)
	{
		VECTOR vertices[2] = {
		    {line_start.x, line_start.y, line_start.z, 1.000f},
		    {line_end.x, line_end.y, line_end.z, 1.000f},
		};

		Matrix local_world = Matrix::UnitMatrix();

		MATRIX local_screen;

		// Now grab our qword pointer and increment past the dmatag.
		qword_t* dmatag = q;
		q++;

		// Create the local_screen matrix.
		create_local_screen(local_screen, local_world.matrix, const_cast<float*>(gs_state.view_world.matrix), const_cast<float*>(gs_state.view_screen.matrix));

		// Calculate the vertex values.
		calculate_vertices(temp_vertices, vertex_count, vertices, local_screen);

		// Convert floating point vertices to fixed point and translate to center of
		// screen.
		draw_convert_xyz(xyz, 2048, 2048, 32, vertex_count,
		                 (vertex_f_t*)temp_vertices);

		// // Draw the triangles using triangle primitive type.
		q = draw_prim_start(q, 0, &prim, &color);

		if (on_top)
		{
			xyz[0].z = 0xffffffff;
			xyz[1].z = 0xffffffff;
		}

		q->dw[0] = xyz[0].xyz;
		q->dw[1] = xyz[1].xyz;
		q++;

		q = draw_prim_end(q, 2, DRAW_XYZ_REGLIST);

		// Define our dmatag for the dma chain.
		DMATAG_CNT(dmatag, q - dmatag - 1, 0, 0, 0);

		return q;
	}

private:
	static constexpr int vertex_count = 2;
	xyz_t* xyz;
	VECTOR* temp_vertices;

	prim_t prim;
} _line_render_proxy;

qword_t* draw_line(qword_t* q, const gs::gs_state& gs_state, const Vector& line_start, const Vector& line_end, color_t color, bool on_top)
{
	return _line_render_proxy.render(q, gs_state, line_start, line_end, color, on_top);
}

void draw_line_one_frame(const Vector& line_start, const Vector& line_end, color_t color, bool on_top)
{
	gs::add_renderable_one_frame([=](qword_t* q, const gs::gs_state& state) -> qword_t* {
		return draw_line(q, state, line_start, line_end, color, on_top);
	});
}

qword_t* draw_plane(qword_t* q, const gs::gs_state& gs_state, const Plane& plane, float size, color_t color, bool on_top)
{
	return draw_line(q, gs_state, plane.get_origin(), plane.get_origin() + (plane.get_normal() * size), color, on_top);
}

void draw_plane_one_frame(const Plane& plane, float size, color_t color, bool on_top)
{
	gs::add_renderable_one_frame([=](qword_t* q, const gs::gs_state& state) -> qword_t* {
		return draw_plane(q, state, plane, size, color, on_top);
	});
}

qword_t* draw_point(qword_t* q, const gs::gs_state& gs_state, const Vector& point, float size, color_t color, bool on_top)
{
	q = draw_line(q, gs_state, point - Vector(size, 0, 0), point + Vector(size, 0, 0), color, on_top);
	q = draw_line(q, gs_state, point - Vector(0, size, 0), point + Vector(0, size, 0), color, on_top);
	q = draw_line(q, gs_state, point - Vector(0, 0, size), point + Vector(0, 0, size), color, on_top);

	return q;
}

void draw_point_one_frame(const Vector& point, float size, color_t color, bool on_top)
{
	gs::add_renderable_one_frame([=](qword_t* q, const gs::gs_state& state) -> qword_t* {
		return draw_point(q, state, point, size, color, on_top);
	});
}
