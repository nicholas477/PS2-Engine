#pragma once

#include "renderer/renderable.hpp"
#include "components/transform_component.hpp"
#include "components/collision_component.hpp"
#include "egg/filesystem.hpp"

class MeshObject: public renderable
{
public:
	MeshObject();
	MeshObject(const Filesystem::Path& mesh_path);
	virtual void on_gs_init() override;
	virtual void render(const gs::gs_state& gs_state) override;
	transform_component transform;
	class Mesh* mesh;
};