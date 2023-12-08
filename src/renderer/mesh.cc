#include "renderer/mesh.hpp"
#include "renderer/renderable.hpp"
#include "renderer/ps2gl_renderers/vertex_color_renderer.hpp"

#include "egg/filesystem.hpp"
#include <egg/mesh_header.hpp>
#include <GL/gl.h>
#include <GL/ps2gl.h>
#include <egg/assert.hpp>

static GLint num_lists = 0;

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
	check(mesh != nullptr);

	printf("Compiling mesh %s, size in bytes: %ld\n", path->data(), mesh->pos.length + mesh->nrm.length);

	list = ++num_lists;
	printf("New mesh draw list: %d\n", list);

	check(num_lists <= 4096);
	glNewList(list, GL_COMPILE);
	{
		glEnable(GL_COLOR_MATERIAL);
		glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
		//glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
		//static float material_diff_amb[] = {0.5f, 0.5f, 0.5f, 0};
		//glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material_diff_amb);
		glShadeModel(GL_SMOOTH);
		glCullFace(GL_BACK);
		//glDisable(GL_LIGHT0);

		//float material[] = {.5f, .5f, .5f, .5f};
		//glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);
		//glFrontFace(GL_CCW);
		//pglEnableCustom(kVCRPrimTypeFlag);

		check((uintptr_t)mesh->pos.get_ptr() % 16 == 0);
		check((uintptr_t)mesh->nrm.get_ptr() % 16 == 0);
		check((uintptr_t)mesh->uvs.get_ptr() % 16 == 0);

		glVertexPointer(4, GL_FLOAT, 0, mesh->pos.get_ptr());
		pglNormalPointer(4, GL_FLOAT, 0, mesh->nrm.get_ptr());

		if (mesh->uvs.offset > 0)
		{
			//glTexCoordPointer(2, GL_FLOAT, 0, mesh->uvs.get_ptr());
		}

		if (mesh->colors.offset > 0)
		{
			//printf("Mesh: %d has colors\n", list);
			glColorPointer(4, GL_FLOAT, 0, mesh->colors.get_ptr());
		}

		int i = 0;
		for (const MeshTriangleStripHeader& strip : mesh->strips)
		{
			//printf("Compiling mesh strip: %d\n", i);
			const auto start_index = strip.strip_start_index;
			const auto count       = strip.strip_end_index - start_index;

			glDrawArrays(mesh->prim_type, start_index, count);

			++i;
		}
	}
	glEndList();
}

void Mesh::draw(bool flush)
{
	if (this == nullptr)
	{
		printf("Mesh::Draw: this is nullptr? wtf\n");
		return;
	}

	if (mesh == nullptr)
	{
		printf("Mesh::draw: Mesh nullptr, not drawing!\n");
		check(false);
		return;
	}

	if (!is_valid())
	{
		printf("Mesh::draw: Mesh not compiled! shame on you!\n");
		compile();
	}


	// {
	// 	static char debug_str[256];
	// 	memset(debug_str, 0, sizeof(debug_str));
	// 	strncpy(debug_str, "Mesh::draw called with an invalid mesh! Did you compile the mesh before drawing it?\n", sizeof(debug_str));
	// 	if (path != nullptr)
	// 	{
	// 		printf("%s\n", path->data());
	// 		printf("list: %d\n", list);
	// 		strncat(debug_str, path->data(), sizeof(debug_str));
	// 		strncat(debug_str, "\n", sizeof(debug_str));
	// 	}

	// 	printf("list >= 0? %s\n", is_valid() ? "TRUE" : "FALSE");
	// 	printf("list num: %d\n", list);
	// 	checkf(is_valid(), debug_str);
	// }

	glCallList(list);
	if (flush)
	{
		glFlush();
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
