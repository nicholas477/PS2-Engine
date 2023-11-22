#include "renderer/mesh.hpp"
#include "renderer/renderable.hpp"

#include "egg/filesystem.hpp"
#include <egg/mesh_header.hpp>
#include <GL/gl.h>
#include <GL/ps2gl.h>
#include <egg/assert.hpp>

//std::unordered_map<Filesystem::Path, Mesh> Mesh::loaded_meshes;
static GLint num_lists = 0;

// static class MeshLoader: public renderable
// {
// public:
// 	MeshLoader()
// 	    : renderable(true)
// 	{
// 	}

// 	virtual void on_gs_init() override
// 	{
// 		//Mesh::loaded_meshes["/assets/models/kettle.ps2_model"_p] = Mesh("/assets/models/kettle.ps2_model"_p);
// 		//Mesh::loaded_meshes["/assets/models/shopping_cart.ps2_model"_p] = Mesh("/assets/models/shopping_cart.ps2_model"_p);

// 		for (auto& [path, mesh] : Mesh::loaded_meshes)
// 		{
// 			mesh.compile();
// 		}
// 	}
// } mesh_loader;

Mesh::Mesh()
{
	list       = -1;
	mesh       = nullptr;
	mesh_bytes = nullptr;
	mesh_size  = 0;
}

Mesh::Mesh(const Filesystem::Path& in_path)
    : Mesh()
{
	path = in_path;
	checkf(Filesystem::load_file(in_path, mesh_bytes, mesh_size, 16), in_path.c_str());
	mesh = reinterpret_cast<MeshFileHeader*>(mesh_bytes.get());
}

void Mesh::compile()
{
	check(!is_valid());
	check(mesh != nullptr);

	printf("Compiling mesh %s, size in bytes: %d\n", path.c_str(), mesh->pos.length + mesh->nrm.length);

	list = num_lists++;
	printf("New mesh draw list: %d\n", list);

	check(num_lists <= 4096);
	glNewList(list, GL_COMPILE);
	{
		glCullFace(GL_BACK);
		//glFrontFace(GL_CCW);

		check((uintptr_t)mesh->pos.get_ptr() % 16 == 0);
		check((uintptr_t)mesh->nrm.get_ptr() % 16 == 0);
		check((uintptr_t)mesh->uvs.get_ptr() % 16 == 0);

		glVertexPointer(4, GL_FLOAT, 0, mesh->pos.get_ptr());
		pglNormalPointer(4, GL_FLOAT, 0, mesh->nrm.get_ptr());
		//glTexCoordPointer(2, GL_FLOAT, 0, mesh->uvs.get_ptr());

		int i = 0;
		for (const MeshTriangleStripHeader& strip : mesh->strips)
		{
			printf("Compiling mesh strip: %d\n", i);
			const auto start_index = strip.strip_start_index;
			const auto count       = strip.strip_end_index - start_index;
			glDrawArrays(GL_TRIANGLE_STRIP, start_index, count);

			++i;
		}
	}
	glEndList();
}

void Mesh::draw(bool flush)
{
	if (this == nullptr)
		return;

	checkf(is_valid(), "Mesh::draw called with an invalid mesh! Did you compile the mesh before drawing it?\n");
	glCallList(list);
	if (flush)
	{
		glFlush();
	}
}
