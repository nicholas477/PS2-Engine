#include "egg/asset.hpp"
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

static class teapot_render_proxy: public Renderable, public Debuggable
{
public:
	teapot_render_proxy()
	    : Renderable(true)
	{
		teapot_mesh = nullptr;
		debug_name  = "teapot render proxy (singleton)";
	}

	virtual void on_gs_init() override
	{
		//teapot_mesh = new Mesh("assets/models/kettle.mdl"_asset);
	}

	virtual void render(const GS::GSState& gs_state) override
	{
		//printf("teapot render proxy rendering. what the fuck is the deal?\n");
	}

	void teapot_render(const GS::GSState& gs_state, const TransformComponent& transform)
	{
		const Matrix local_world = Matrix::from_location_and_rotation(transform.get_location(), transform.get_rotation());

		ScopedMatrix sm(local_world);

		//printf("Drawing teapot\n");
		//teapot_mesh->draw(false);
	}

	AABB get_bounds() const
	{
		// Cache the bounds
		static AABB out = compute_bounds();
		return out;
	}


	virtual const char* get_type_name() const { return typeid(teapot_render_proxy).name(); }

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

Teapot::Teapot()
    : collision(&transform)
{
	render_proxy = &_teapot_render_proxy;

	collision.set_local_bounds(_teapot_render_proxy.get_bounds());
}

void Teapot::render(const GS::GSState& gs_state)
{
	render_proxy->teapot_render(gs_state, transform);
}