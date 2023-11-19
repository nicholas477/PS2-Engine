#include "renderer/mesh.hpp"

#include "utils/filesystem.hpp"
#include <egg/mesh_header.hpp>
#include <GL/gl.h>
#include <GL/ps2gl.h>
#include <assert.hpp>

static GLint num_lists = 0;

Mesh::Mesh()
{
	list       = -1;
	mesh       = nullptr;
	mesh_bytes = nullptr;
	mesh_size  = 0;
}

Mesh::Mesh(const Filesystem::Path& path)
    : Mesh()
{
	check(Filesystem::load_file(path, mesh_bytes, mesh_size, 16));
	mesh = reinterpret_cast<MeshFileHeader*>(mesh_bytes.get());
}

void Mesh::compile()
{
	check(!is_valid());
	check(mesh != nullptr);

	printf("Compiling mesh, size in bytes: %d\n", mesh_size);

	list = num_lists++;
	check(num_lists <= 4096);
	glNewList(list, GL_COMPILE);
	{
		glCullFace(GL_BACK);
		//glFrontFace(GL_CCW);

		glVertexPointer(4, GL_FLOAT, 0, mesh->pos.get_ptr());
		pglNormalPointer(4, GL_FLOAT, 0, mesh->nrm.get_ptr());
		glTexCoordPointer(2, GL_FLOAT, 0, mesh->uvs.get_ptr());

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

void Mesh::draw()
{
	check(is_valid());
	glCallList(list);
}
