#include "objects/teapot.hpp"
#include "utils/rendering.hpp"
#include "renderer/mesh.hpp"
#include <malloc.h>
#include <stdio.h>

#include <draw.h>
#include <draw3d.h>

#include <graph.h>
#include <dma_tags.h>
#include <gif_tags.h>
#include <gs_psm.h>

#include <GL/gl.h> // The GL Header File
#include <dma.h>

static class teapot_render_proxy
{
public:
	teapot_render_proxy()
	{
	}

	void on_gs_init()
	{
		teapot_mesh = new Mesh("/assets/models/kettle.ps2_model"_p); //&Mesh::loaded_meshes["/assets/models/kettle.ps2_model"_p];
		                                                             //teapot_mesh = nullptr;
		teapot_mesh->compile();
	}

	void render(const gs::gs_state& gs_state, const transform_component& transform)
	{
		const Matrix local_world = Matrix::from_location_and_rotation(transform.get_location(), transform.get_rotation());

		ScopedMatrix sm(local_world * gs_state.world_view);

		static float ps2_diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
		static float black[]       = {0, 0, 0, 0};
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, ps2_diffuse);
		glMaterialfv(GL_FRONT, GL_EMISSION, black);

		teapot_mesh->draw();
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
		// out.Min = vertices[0];
		// out.Max = vertices[0];
		// for (int i = 0; i < vertex_count; ++i)
		// {
		// 	out.Min.x = std::min(out.Min.x, vertices[i][0]);
		// 	out.Min.y = std::min(out.Min.y, vertices[i][1]);
		// 	out.Min.z = std::min(out.Min.z, vertices[i][2]);

		// 	out.Max.x = std::max(out.Max.x, vertices[i][0]);
		// 	out.Max.y = std::max(out.Max.y, vertices[i][1]);
		// 	out.Max.z = std::max(out.Max.z, vertices[i][2]);
		// }

		return out;
	}

	Mesh* teapot_mesh;
} _teapot_render_proxy;

static class teapot_render_proxy_initializer: public renderable
{
public:
	teapot_render_proxy_initializer()
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

	// color_t color;
	// color.r = 32;
	// color.g = 32;
	// color.b = 32;
	// color.a = 128.f;
	// color.q = 1.f;

	// collision.render(gs_state, color);
}