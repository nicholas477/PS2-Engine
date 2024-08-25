#include "renderer/mesh.hpp"
#include "renderer/renderable.hpp"
#include "renderer/vu1/vu_programs.hpp"

#include "egg/filesystem.hpp"
#include <egg/mesh_header.hpp>
#include <egg/assert.hpp>

#include "egg-ps2-graphics-lib/egg-ps2-graphics-lib.hpp"
#include "egg-ps2-graphics-lib/mesh.hpp"

Mesh::Mesh()
{
	mesh_asset = nullptr;
	path       = nullptr;
	debug_name = "uninitialized mesh";
}

Mesh::Mesh(Asset::Reference mesh_asset_ref)
    : Mesh()
{
	load_from_asset_ref(mesh_asset_ref);
	debug_name = Asset::lookup_path(mesh_asset_ref).data();
}

void Mesh::load_from_asset_ref(Asset::Reference mesh_asset_ref)
{
	check(AssetRegistry::get_asset(mesh_asset_ref, mesh_asset, 16, true));
}

void Mesh::draw(const GS::GSState& gs_state, const Matrix& render_matrix, bool flush)
{
	using namespace egg::ps2::graphics;
	if (get_mesh() == nullptr)
	{
		printf("Mesh::draw: Mesh nullptr, not drawing!\n");
		return;
	}

	for (const MeshTriangleStripHeader& strip : get_mesh()->strips)
	{
		const auto start_index = strip.strip_start_index;
		const auto end_index   = strip.strip_end_index;

		mesh_descriptor m;
		m.pos             = get_mesh()->pos.get_ptr() + start_index;
		m.color           = get_mesh()->colors.get_ptr() + start_index;
		m.num_verts       = end_index - start_index;
		m.vu_program_addr = get_vertex_color_program_addr();
		m.enable_fog      = true;

		m.enable_texture_mapping = false;

		m.set_fog_start_and_end(gs_state.fog_start_end.first, gs_state.fog_start_end.second);

		draw_mesh_strip(render_matrix, m);
	}
}

int Mesh::get_triangle_count() const
{
	int triangle_count = 0;

	if (get_mesh())
	{
		for (const MeshTriangleStripHeader& strip : get_mesh()->strips)
		{
			triangle_count += (strip.strip_end_index - strip.strip_start_index) - 2;
		}
	}

	return triangle_count;
}

MeshFileHeader* Mesh::get_mesh() const
{
	check(mesh_asset != nullptr);
	return reinterpret_cast<MeshFileHeader*>(mesh_asset->data.get());
}
