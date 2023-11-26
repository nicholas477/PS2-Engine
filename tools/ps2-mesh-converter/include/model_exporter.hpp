#pragma once
#include "egg/math_types.hpp"
#include <vector>

struct MeshStrip
{
	std::vector<Vector> positions;
	std::vector<Vector> normals;
	std::vector<Vector2> texture_coords;
	std::vector<Vector> colors;
};

std::vector<std::byte> serialize_meshes(const std::vector<MeshStrip>& strips);