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
	mesh_bytes   = nullptr;
	mesh_size    = 0;
	path         = nullptr;
	auto_compile = false;
	debug_name   = "uninitialized mesh";
}

Mesh::Mesh(const Filesystem::Path& in_path)
    : Mesh()
{
	load_from_path(in_path);
	debug_name = in_path.data();
}

Mesh::Mesh(Asset::Reference mesh_asset)
    : Mesh(Asset::lookup_path(mesh_asset))
{
}

void Mesh::load_from_path(const Filesystem::Path& in_path)
{
	path = &in_path;
	checkf(path->length() > 0, path->data());
	checkf(Filesystem::load_file(in_path, mesh_bytes, mesh_size, 16), in_path.data());

	check(__is_aligned(mesh_bytes.get(), 16));
}

void Mesh::load_from_asset_ref(Asset::Reference mesh_asset)
{
	load_from_path(Asset::lookup_path(mesh_asset));
}


void Mesh::draw(const Matrix& render_matrix, bool flush)
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
	return reinterpret_cast<MeshFileHeader*>(mesh_bytes.get());
}
