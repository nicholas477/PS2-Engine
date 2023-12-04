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

	Mesh(const Filesystem::Path& path, bool in_auto_compile = true);
	Mesh(Asset::Reference mesh_asset, bool in_auto_compile = true);

	int list;
	class MeshFileHeader* mesh;
	std::unique_ptr<std::byte[]> mesh_bytes;
	size_t mesh_size;

	void compile();
	void draw(bool flush = false);

	bool is_valid() const { return mesh != nullptr && list >= 0; }
	int get_triangle_count() const;

	virtual void on_gs_init() override;

	// for debug
	const Filesystem::Path* path;
	bool auto_compile;
	virtual const char* get_type_name() const { return typeid(Mesh).name(); }
};