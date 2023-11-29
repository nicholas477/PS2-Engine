#pragma once

#include "offset_pointer.hpp"
#include "serialization.hpp"
#include "math_types.hpp"
#include <cstdint>
#include <vector>

#pragma pack(push, 1)

struct MeshTriangleStripHeader
{
	uint32_t strip_start_index; // Index of the starting vertex
	uint32_t strip_end_index;   // Index of the ending vertex
};

struct MeshFileHeader
{
	OffsetArray<Vector> pos;
	OffsetArray<Vector> nrm;
	OffsetArray<Vector2> uvs;
	OffsetArray<Vector> colors;

	// The strips are start/end indicies into pos/nrm/uvs
	OffsetArray<MeshTriangleStripHeader> strips;
};

static_assert(std::is_standard_layout_v<Vector> == true);
static_assert(std::is_standard_layout_v<OffsetArray<Vector>> == true);
static_assert(std::is_standard_layout_v<OffsetArray<Vector2>> == true);
static_assert(std::is_standard_layout_v<OffsetArray<MeshTriangleStripHeader>> == true);
static_assert(std::is_standard_layout_v<MeshFileHeader> == true);

#pragma pack(pop)