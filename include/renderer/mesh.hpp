#pragma once

#include <egg/filesystem.hpp>
#include <egg/asset.hpp>
#include "egg/math_types.hpp"
#include "renderer/renderable.hpp"
#include "utils/debuggable.hpp"
#include <unordered_map>
#include <vector>
#include <memory>

#include "asset/asset_registry.hpp"

class Mesh: public Renderable, public Debuggable
{
public:
	Mesh();

	Mesh(Asset::Reference mesh_asset);

	AssetRegistry::Asset* mesh_asset;
	AssetRegistry::Asset* texture_asset;

	class MeshFileHeader* get_mesh() const;

	// Can be nullptr if there is no texture
	class TextureFileHeader* get_texture() const;

	void load_from_asset_ref(Asset::Reference mesh_asset);

	void draw(const GS::GSState& gs_state, const Matrix& render_matrix, bool flush = false);

	bool is_valid() const { return get_mesh() != nullptr; }
	int get_triangle_count() const;

	// for debug
	const Filesystem::Path* path;
	virtual const char* get_type_name() const { return typeid(Mesh).name(); }

	// Copy constructor (no copying!)
	Mesh(const Mesh&) = delete;
};