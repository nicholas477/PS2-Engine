#pragma once

#include "tick.hpp"
#include "renderer/renderable.hpp"
#include "utils/debuggable.hpp"
#include "components/transform_component.hpp"
#include "components/collision_component.hpp"

#include "objects/mesh_object.hpp"

class Teapot: public RootComponentInterface
{
public:
	Teapot();
	collision_component collision;

	MeshObject teapot_mesh;

	virtual TransformComponent* get_root_component() { return teapot_mesh.get_root_component(); }
};