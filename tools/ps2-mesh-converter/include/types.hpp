#pragma once

#include <vector>

#include <egg/mesh_header.hpp>

struct Vertex
{
	float px, py, pz;
	float nx, ny, nz;
	float tx, ty;
	float r, g, b, a;
};

struct Mesh
{
	uint32_t primitive_type;
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
};

static bool isMeshValid(const Mesh& mesh)
{
	size_t index_count  = mesh.indices.size();
	size_t vertex_count = mesh.vertices.size();

	if (index_count % 3 != 0)
		return false;

	const unsigned int* indices = &mesh.indices[0];

	for (size_t i = 0; i < index_count; ++i)
		if (indices[i] >= vertex_count)
			return false;

	return true;
}

union Triangle
{
	Triangle()
	{
	}
	Vertex v[3];
	char data[sizeof(Vertex) * 3];
};

static bool ichar_equals(char a, char b)
{
	return std::tolower(static_cast<unsigned char>(a)) ==
	       std::tolower(static_cast<unsigned char>(b));
}

// Case insensitive string compare
static bool iequals(const std::string& a, const std::string& b)
{
	return std::equal(a.begin(), a.end(), b.begin(), b.end(), ichar_equals);
}