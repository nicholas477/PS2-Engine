#include "renderer/mesh.hpp"

static GLint num_lists = 0;

mesh::mesh(const std::string& path)
{
	ps2glMeshHeader* mesh;

	list = num_lists++;
	glNewList(list, GL_COMPILE);
	{
	}
	glEndList();
}