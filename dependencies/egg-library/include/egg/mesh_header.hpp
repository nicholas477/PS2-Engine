#pragma once

#include "offset_pointer.hpp"
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

	// The strips are start/end indicies into pos/nrm/uvs
	OffsetArray<MeshTriangleStripHeader> strips;
};

#pragma pack(pop)