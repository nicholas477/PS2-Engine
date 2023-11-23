#pragma once
#include "egg/math_types.hpp"
#include <vector>

std::vector<std::byte> serialize_mesh(const std::vector<Vector>& positions,
                                      const std::vector<Vector>& normals,
                                      const std::vector<Vector2>& texture_coords,
                                      const std::vector<Vector>& colors);