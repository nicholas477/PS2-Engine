#pragma once

#include <utils/filesystem.hpp>
#include "egg/math_types.hpp"
#include <unordered_map>
#include <vector>
#include <memory>

class Mesh
{
public:
	Mesh();

	Mesh(const Filesystem::Path& path);

	int list;
	class MeshFileHeader* mesh;
	std::unique_ptr<std::byte[]> mesh_bytes;
	size_t mesh_size;

	void compile();
	void draw();

	bool is_valid() const { return list >= 0; }

	static std::unordered_map<Filesystem::Path, Mesh> loaded_meshes;
};