#pragma once
#include "egg/math_types.hpp"
#include "types.hpp"
#include <cstdint>
#include <vector>

std::vector<std::byte> serialize_meshes(uint32_t prim_type, const std::vector<MeshStrip>& strips);