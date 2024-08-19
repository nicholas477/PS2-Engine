#pragma once

#include "renderer/renderable.hpp"
#include "utils/debuggable.hpp"
#include "components/transform_component.hpp"
#include "components/collision_component.hpp"
#include "egg/filesystem.hpp"

class MeshObject: public Renderable, public Debuggable, public RootComponentInterface
{
public:
	MeshObject();
	MeshObject(const Filesystem::Path& mesh_path);
	MeshObject(Asset::Reference mesh_reference);
	virtual void render(const GS::GSState& gs_state) override;
	TransformComponent transform;
	class Mesh* mesh;

	virtual const char* get_name() const override;
	virtual const char* get_type_name() const { return typeid(MeshObject).name(); }

	virtual TransformComponent* get_root_component() { return &transform; }
};