#include "renderer/mesh.hpp"
#include "renderer/renderable.hpp"

#include "egg/filesystem.hpp"
#include <egg/mesh_header.hpp>
#include <egg/assert.hpp>

#include "egg-ps2-graphics-lib/egg-ps2-graphics-lib.hpp"
#include "egg-ps2-graphics-lib/texture.hpp"
#include "egg-ps2-graphics-lib/mesh.hpp"

Mesh::Mesh()
{
	mesh_asset = nullptr;
	texture    = nullptr;
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

	// Load the texture too (if specified)
	if (get_mesh()->texture != Asset::Reference())
	{
		texture = get_texture(get_mesh()->texture);
		printf("Loaded texture!\n");
		check(texture != nullptr);
	}
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
		m.pos        = get_mesh()->pos.get_ptr() + start_index;
		m.color      = get_mesh()->colors.get_ptr() + start_index;
		m.num_verts  = end_index - start_index;
		m.enable_fog = true;

		if (texture != nullptr)
		{
			if (get_mesh()->uvs.length == 0 || get_mesh()->uvs.get_ptr() == nullptr)
			{
				fprintf(stderr, "Warning! Mesh %s has texture set but no UVs!\n", debug_name.c_str());
				return;
			}

			m.uvs     = get_mesh()->uvs.get_ptr() + start_index;
			m.texture = &texture->texture_descriptor;

			if (!texture->texture_descriptor.is_uploaded)
			{
				printf("Uploading texture!\n");
				check(texture->upload_texture());
			}

			// printf("Using texture!\n");
			// check(texture->use_texture());
		}
		else
		{
			m.uvs = nullptr;
		}

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