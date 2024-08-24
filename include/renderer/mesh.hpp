#pragma once

#include <egg/filesystem.hpp>
#include <egg/asset.hpp>
#include "egg/math_types.hpp"
#include "renderer/renderable.hpp"
#include "utils/debuggable.hpp"
#include <unordered_map>
#include <vector>
#include <memory>

class Mesh: public Renderable, public Debuggable
{
public:
	Mesh();

	Mesh(const Filesystem::Path& path);
	Mesh(Asset::Reference mesh_asset);

	class MeshFileHeader* get_mesh() const;
	std::unique_ptr<std::byte[]> mesh_bytes;
	size_t mesh_size;

	void load_from_path(const Filesystem::Path& path);
	void load_from_asset_ref(Asset::Reference mesh_asset);

	void draw(const GS::GSState& gs_state, const Matrix& render_matrix, bool flush = false);

	bool is_valid() const { return get_mesh() != nullptr; }
	int get_triangle_count() const;

	// for debug
	const Filesystem::Path* path;
	bool auto_compile;
	virtual const char* get_type_name() const { return typeid(Mesh).name(); }

	// Copy constructor (no copying!)
	Mesh(const Mesh&) = delete;
};