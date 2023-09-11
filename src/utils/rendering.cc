#include "utils/rendering.hpp"

#include "draw.h"

#include <dma_tags.h>
#include <gif_tags.h>
#include <gs_psm.h>

#include <malloc.h>
#include <stdio.h>

#include <font.h>
#include <dma.h>

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

	void render(const gs::gs_state& gs_state, const AABB& aabb, color_t color, bool on_top)
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

		// Create the local_screen matrix.
		create_local_screen(local_screen, local_world.matrix, const_cast<float*>(gs_state.world_view.matrix), const_cast<float*>(gs_state.view_screen.matrix));

		// Calculate the vertex values.
		calculate_vertices(temp_vertices, vertex_count, vertices, local_screen);

		// Convert floating point vertices to fixed point and translate to center of
		// screen.
		draw_convert_xyz(xyz, 2048, 2048, 32, vertex_count,
		                 (vertex_f_t*)temp_vertices);

		packet2 packet = packet2(16, Packet2Type::P2_TYPE_NORMAL, Packet2Mode::P2_MODE_NORMAL, false);
		// Draw the triangles using triangle primitive type.
		packet.update(draw_prim_start, 0, &prim, &color);

		if (on_top)
		{
			for (int i = 0; i < vertex_count; ++i)
			{
				xyz[i].z = 0xffffffff;
			}
		}

		// Bottom rectangle (-y)
		{
			packet.add(xyz[0].xyz);
			packet.add(xyz[1].xyz);

			packet.add(xyz[0].xyz);
			packet.add(xyz[5].xyz);

			packet.add(xyz[7].xyz);
			packet.add(xyz[1].xyz);

			packet.add(xyz[5].xyz);
			packet.add(xyz[7].xyz);
		}

		// Top rectangle (+y)
		{
			packet.add(xyz[6].xyz);
			packet.add(xyz[2].xyz);

			packet.add(xyz[6].xyz);
			packet.add(xyz[4].xyz);

			packet.add(xyz[3].xyz);
			packet.add(xyz[2].xyz);

			packet.add(xyz[4].xyz);
			packet.add(xyz[3].xyz);
		}

		// Top/bottom edges
		{
			packet.add(xyz[0].xyz);
			packet.add(xyz[6].xyz);

			packet.add(xyz[4].xyz);
			packet.add(xyz[5].xyz);

			packet.add(xyz[7].xyz);
			packet.add(xyz[3].xyz);

			packet.add(xyz[1].xyz);
			packet.add(xyz[2].xyz);
		}

		packet.update(draw_prim_end, 2, DRAW_XYZ_REGLIST);
		dma_channel_wait(DMA_CHANNEL_GIF, 0);
		dma_channel_send_packet2(packet, DMA_CHANNEL_GIF, 1);
	}

private:
	static constexpr int vertex_count = 8;
	xyz_t* xyz;
	VECTOR* temp_vertices;

	prim_t prim;
} _aabb_render_proxy;

void draw_aabb(const gs::gs_state& gs_state, const AABB& aabb, color_t color, bool on_top)
{
	_aabb_render_proxy.render(gs_state, aabb, color, on_top);
}

void draw_aabb_one_frame(const AABB& aabb, color_t color, bool on_top)
{
	gs::add_renderable_one_frame([=](const gs::gs_state& state) {
		draw_aabb(state, aabb, color, on_top);
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

	void render(const gs::gs_state& gs_state, const Vector& line_start, const Vector& line_end, color_t color, bool on_top)
	{
		VECTOR vertices[2] = {
		    {line_start.x, line_start.y, line_start.z, 1.000f},
		    {line_end.x, line_end.y, line_end.z, 1.000f},
		};

		Matrix local_world = Matrix::UnitMatrix();

		MATRIX local_screen;

		// Create the local_screen matrix.
		create_local_screen(local_screen, local_world.matrix, const_cast<float*>(gs_state.world_view.matrix), const_cast<float*>(gs_state.view_screen.matrix));

		// Calculate the vertex values.
		calculate_vertices(temp_vertices, vertex_count, vertices, local_screen);

		// Convert floating point vertices to fixed point and translate to center of
		// screen.
		draw_convert_xyz(xyz, 2048, 2048, 32, vertex_count,
		                 (vertex_f_t*)temp_vertices);

		packet2 packet = packet2(8, Packet2Type::P2_TYPE_NORMAL, Packet2Mode::P2_MODE_NORMAL, false);
		// Draw the triangles using triangle primitive type.
		packet.update(draw_prim_start, 0, &prim, &color);

		if (on_top)
		{
			xyz[0].z = 0xffffffff;
			xyz[1].z = 0xffffffff;
		}

		packet.add(xyz[0].xyz);
		packet.add(xyz[1].xyz);

		packet.update(draw_prim_end, 2, DRAW_XYZ_REGLIST);
		dma_channel_wait(DMA_CHANNEL_GIF, 0);
		dma_channel_send_packet2(packet, DMA_CHANNEL_GIF, 1);
	}

private:
	static constexpr int vertex_count = 2;
	xyz_t* xyz;
	VECTOR* temp_vertices;

	prim_t prim;
} _line_render_proxy;

void draw_line(const gs::gs_state& gs_state, const Vector& line_start, const Vector& line_end, color_t color, bool on_top)
{
	_line_render_proxy.render(gs_state, line_start, line_end, color, on_top);
}

void draw_line_one_frame(const Vector& line_start, const Vector& line_end, color_t color, bool on_top)
{
	gs::add_renderable_one_frame([=](const gs::gs_state& state) {
		draw_line(state, line_start, line_end, color, on_top);
	});
}

void draw_plane(const gs::gs_state& gs_state, const Plane& plane, float size, color_t color, bool on_top)
{
	// Arrow pointing out
	draw_line(gs_state, plane.get_origin(), plane.get_origin() + (plane.get_normal() * size), color, on_top);

	Vector U, V;
	plane.find_best_axis_vectors(U, V);
	U *= size;
	V *= size;
	Vector plane_rects[4] = {
	    plane.get_origin() + U + V,
	    plane.get_origin() - U + V,
	    plane.get_origin() + U - V,
	    plane.get_origin() - U - V};

	draw_line(gs_state, plane_rects[0], plane_rects[1], color, on_top);
	draw_line(gs_state, plane_rects[1], plane_rects[2], color, on_top);
	draw_line(gs_state, plane_rects[2], plane_rects[3], color, on_top);
	draw_line(gs_state, plane_rects[3], plane_rects[0], color, on_top);
}

void draw_plane_one_frame(const Plane& plane, float size, color_t color, bool on_top)
{
	gs::add_renderable_one_frame([=](const gs::gs_state& state) {
		return draw_plane(state, plane, size, color, on_top);
	});
}

void draw_point(const gs::gs_state& gs_state, const Vector& point, float size, color_t color, bool on_top)
{
	draw_line(gs_state, point - Vector(size, 0, 0), point + Vector(size, 0, 0), color, on_top);
	draw_line(gs_state, point - Vector(0, size, 0), point + Vector(0, size, 0), color, on_top);
	draw_line(gs_state, point - Vector(0, 0, size), point + Vector(0, 0, size), color, on_top);
}

void draw_point_one_frame(const Vector& point, float size, color_t color, bool on_top)
{
	gs::add_renderable_one_frame([=](const gs::gs_state& state) {
		draw_point(state, point, size, color, on_top);
	});
}

void draw_orientation_gizmo(const gs::gs_state& gs_state, const Vector& pos, float size, bool on_top)
{
	draw_line(gs_state, pos, pos + Vector(size, 0, 0), colors::red(), on_top);   // x
	draw_line(gs_state, pos, pos + Vector(0, size, 0), colors::green(), on_top); // y
	draw_line(gs_state, pos, pos + Vector(0, 0, size), colors::blue(), on_top);  // z
}

void draw_orientation_gizmo_one_frame(const Vector& pos, float size, bool on_top)
{
	gs::add_renderable_one_frame([=](const gs::gs_state& state) {
		draw_orientation_gizmo(state, pos, size, on_top);
	});
}

void draw_text(const gs::gs_state& gs_state, const Vector& pos, float size, const std::string& text)
{
	// implement this shit
}