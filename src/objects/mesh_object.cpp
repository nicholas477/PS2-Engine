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
		const Matrix local_world = Matrix::from_location_and_rotation(transform.get_location(), transform.get_rotation());
		ScopedMatrix sm(local_world);

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