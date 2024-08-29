#include "objects/mesh_object.hpp"

#include "renderer/mesh.hpp"

MeshObject::MeshObject()
{
	mesh = nullptr;
}

MeshObject::MeshObject(Asset::Reference mesh_reference)
{
	mesh = new Mesh(mesh_reference);
}

void MeshObject::render(const GS::GSState& gs_state)
{
	if (mesh)
	{
		mesh->draw(gs_state, transform.get_matrix() * gs_state.world_screen);
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