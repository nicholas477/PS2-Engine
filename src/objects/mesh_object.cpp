#include "objects/mesh_object.hpp"

#include "renderer/mesh.hpp"

MeshObject::MeshObject()
{
	mesh = nullptr;
}

MeshObject::MeshObject(const Filesystem::Path& mesh_path)
{
	mesh = new Mesh(mesh_path);
}

MeshObject::MeshObject(Asset::Reference mesh_reference)
    : MeshObject(Asset::lookup_path(mesh_reference))
{
}

void MeshObject::on_gs_init()
{
	if (mesh)
	{
		mesh->compile();
	}
}

void MeshObject::render(const GS::GSState& gs_state)
{
	if (mesh)
	{
		ScopedMatrix sm(transform.get_matrix());
		mesh->draw();
	}
}

const char* MeshObject::get_name() const
{
	if (mesh)
	{
		return mesh->get_name();
	}
	else
	{
		return debug_name.c_str();
	}
}