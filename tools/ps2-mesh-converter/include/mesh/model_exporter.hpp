#pragma once
#include "egg/math_types.hpp"
#include <cstdint>
#include <vector>

struct MeshStrip
{
	std::vector<Vector> positions;
	std::vector<Vector> normals;
	std::vector<Vector2> texture_coords;
	std::vector<Vector> colors;
};

std::vector<std::byte> serialize_meshes(uint32_t prim_type, const std::vector<MeshStrip>& strips);