#include "renderer/mesh.hpp"
#include "renderer/renderable.hpp"

#include "egg/filesystem.hpp"
#include <egg/mesh_header.hpp>
#include <egg/assert.hpp>

#include "egg-ps2-graphics-lib/egg-ps2-graphics-lib.hpp"

//static GLint num_lists = 0;

Mesh::Mesh()
{
	list         = -1;
	mesh         = nullptr;
	mesh_bytes   = nullptr;
	mesh_size    = 0;
	path         = nullptr;
	auto_compile = false;
	debug_name   = "uninitialized mesh";
}

Mesh::Mesh(const Filesystem::Path& in_path, bool in_auto_compile)
    : Mesh()
{
	path = &in_path;
	checkf(path->length() > 0, path->data());
	checkf(Filesystem::load_file(in_path, mesh_bytes, mesh_size, 16), in_path.data());
	debug_name = in_path.data();
	mesh       = reinterpret_cast<MeshFileHeader*>(mesh_bytes.get());

	auto_compile = in_auto_compile;
	if (auto_compile && GS::has_gs_initialized())
	{
		compile();
	}
}

Mesh::Mesh(Asset::Reference mesh_asset, bool in_auto_compile)
    : Mesh(Asset::lookup_path(mesh_asset), in_auto_compile)
{
}

void Mesh::on_gs_init()
{
	if (auto_compile)
	{
		compile();
	}
}

void Mesh::compile()
{
	if (is_valid())
	{
		printf("Trying to compile already compiled mesh!\nMesh list: %d\n", list);
		return;
	}

	check(!is_valid());

	if (mesh == nullptr)
	{
		printf("Trying to compile null mesh!\n");
		return;
	}
}

void Mesh::draw(const Matrix& render_matrix, bool flush)
{
	if (mesh == nullptr)
	{
		printf("Mesh::draw: Mesh nullptr, not drawing!\n");
		return;
	}

	for (const MeshTriangleStripHeader& strip : mesh->strips)
	{
		const auto start_index = strip.strip_start_index;
		const auto end_index   = strip.strip_end_index;
		egg::ps2::graphics::draw_mesh(render_matrix, end_index - start_index, mesh->pos.get_ptr() + start_index);
	}
}

int Mesh::get_triangle_count() const
{
	int triangle_count = 0;

	if (mesh)
	{
		for (const MeshTriangleStripHeader& strip : mesh->strips)
		{
			triangle_count += (strip.strip_end_index - strip.strip_start_index) - 2;
		}
	}

	return triangle_count;
}
