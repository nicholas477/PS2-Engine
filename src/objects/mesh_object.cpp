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

void MeshObject::on_gs_init()
{
	if (mesh)
	{
		mesh->compile();
	}
}

void MeshObject::render(const gs::gs_state& gs_state)
{
	if (mesh)
	{
		const Matrix local_world = Matrix::from_location_and_rotation(transform.get_location(), transform.get_rotation());
		ScopedMatrix sm(local_world); // * gs_state.world_view);

		mesh->draw();
	}
}