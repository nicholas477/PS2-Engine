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
	//check(mesh != nullptr);

	if (mesh == nullptr)
	{
		printf("Trying to compile null mesh!\n");
		return;
	}

	// printf("Compiling mesh %s, size in bytes: %ld\n", path->data(), mesh->pos.length + mesh->nrm.length);

	// list = ++num_lists;
	// printf("New mesh draw list: %d\n", list);

	// check(num_lists <= 4096);
	// glNewList(list, GL_COMPILE);
	// {
	// 	glEnable(GL_COLOR_MATERIAL);
	// 	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
	// 	//glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	// 	//static float material_diff_amb[] = {0.5f, 0.5f, 0.5f, 0};
	// 	//glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material_diff_amb);
	// 	glShadeModel(GL_SMOOTH);
	// 	glCullFace(GL_BACK);
	// 	//glDisable(GL_LIGHT0);

	// 	//float material[] = {.5f, .5f, .5f, .5f};
	// 	//glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);
	// 	//glFrontFace(GL_CCW);
	// 	//pglEnableCustom(kVCRPrimTypeFlag);

	// 	check((uintptr_t)mesh->pos.get_ptr() % 16 == 0);
	// 	check((uintptr_t)mesh->nrm.get_ptr() % 16 == 0);
	// 	check((uintptr_t)mesh->uvs.get_ptr() % 16 == 0);

	// 	check(mesh->pos.offset > 0);

	// glVertexPointer(4, GL_FLOAT, 0, mesh->pos.get_ptr());

	// 	if (mesh->nrm.offset > 0)
	// 	{
	// 		pglNormalPointer(4, GL_FLOAT, 0, mesh->nrm.get_ptr());
	// 	}

	// 	if (mesh->uvs.offset > 0)
	// 	{
	// 		glTexCoordPointer(2, GL_FLOAT, 0, mesh->uvs.get_ptr());
	// 	}

	// 	if (mesh->colors.offset > 0)
	// 	{
	// 		glColorPointer(4, GL_FLOAT, 0, mesh->colors.get_ptr());
	// 	}

	// 	int i = 0;
	// 	for (const MeshTriangleStripHeader& strip : mesh->strips)
	// 	{
	// 		//printf("Compiling mesh strip: %d\n", i);
	// 		const auto start_index = strip.strip_start_index;
	// 		const auto count       = strip.strip_end_index - start_index;

	// 		glDrawArrays(mesh->prim_type, start_index, count);

	// 		++i;
	// 	}
	// }
	// glEndList();
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
		for (int32_t i = start_index; i <= end_index; i += 64)
		{
			const auto i_start = std::max(i - 2, (int32_t)start_index);
			const auto i_end   = std::min((uint32_t)i + 64, end_index);
			assert(i_start != i_end);
			//printf("Start index: %d\n", i_start);
			//printf("End index: %d\n", i_end);

			egg::ps2::graphics::draw_mesh(render_matrix, i_end - i_start, mesh->pos.get_ptr() + i_start, mesh->colors.get_ptr() + i_start);
		}
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
