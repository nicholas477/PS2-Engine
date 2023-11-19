#pragma once

#include <utils/filesystem.hpp>
#include "egg/math_types.hpp"
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
};