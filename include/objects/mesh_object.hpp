#pragma once

#include "renderer/renderable.hpp"
#include "components/transform_component.hpp"
#include "components/collision_component.hpp"
#include "egg/filesystem.hpp"

class MeshObject: public Renderable
{
public:
	MeshObject();
	MeshObject(const Filesystem::Path& mesh_path);
	virtual void on_gs_init() override;
	virtual void render(const GS::GSState& gs_state) override;
	TransformComponent transform;
	class Mesh* mesh;
};