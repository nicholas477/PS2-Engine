#pragma once

#include <utils/filesystem.hpp>
#include "egg/math_types.hpp"
#include <vector>

// struct ps2glMeshHeader
// {
// 	int nStrips;
// };

// struct ps2glStripHeader
// {
// 	int nVertices;
// 	int nNormals;
// 	int nUVs;
// 	int nRGBAs;
// };

// struct ps2glMeshVertex
// {
// 	float x;
// 	float y;
// 	float z;
// };

// struct ps2glMeshNormal
// {
// 	float nx;
// 	float ny;
// 	float nz;
// };

// struct ps2glMeshRGBA
// {
// 	float r;
// 	float g;
// 	float b;
// 	float a;
// };

struct ps2glMeshUV
{
	float u;
	float v;
};

class Mesh
{
public:
	Mesh();

	Mesh(const Filesystem::Path& path);

	int list;
	class MeshFileHeader* mesh;

	void compile();
	void draw();

	bool is_valid() const { return list >= 0; }
};