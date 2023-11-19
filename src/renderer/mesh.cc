#include "renderer/mesh.hpp"

#include "utils/filesystem.hpp"
#include <egg/mesh_header.hpp>
#include <GL/gl.h>
#include <GL/ps2gl.h>
#include <assert.hpp>

static GLint num_lists = 0;

Mesh::Mesh()
{
	list = -1;
	mesh = nullptr;
}

Mesh::Mesh(const Filesystem::Path& path)
    : Mesh()
{
	std::unique_ptr<std::byte[]> mesh_bytes;
	check(Filesystem::load_file(path, mesh_bytes, 16));
	mesh = reinterpret_cast<MeshFileHeader*>(mesh_bytes.get());
}

void Mesh::compile()
{
	check(!is_valid());
	check(mesh != nullptr);

	list = num_lists++;
	check(num_lists <= 4096);
	glNewList(list, GL_COMPILE);
	{
		glVertexPointer(4, GL_FLOAT, 0, mesh->pos.get_ptr());
		pglNormalPointer(4, GL_FLOAT, 0, mesh->nrm.get_ptr());
		glTexCoordPointer(2, GL_FLOAT, 0, mesh->uvs.get_ptr());

		const auto start_index = mesh->strips[0].strip_start_index;
		const auto count       = mesh->strips[0].strip_end_index - start_index;
		glDrawArrays(GL_TRIANGLE_STRIP, start_index, count);
	}
	glEndList();
}

void Mesh::draw()
{
	check(is_valid());
	glCallList(list);
}
