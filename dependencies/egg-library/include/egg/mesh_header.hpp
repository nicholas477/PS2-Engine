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

	uint32_t prim_type;
};

static_assert(std::is_standard_layout_v<Vector> == true);
static_assert(std::is_standard_layout_v<OffsetArray<Vector>> == true);
static_assert(std::is_standard_layout_v<OffsetArray<Vector2>> == true);
static_assert(std::is_standard_layout_v<OffsetArray<MeshTriangleStripHeader>> == true);
static_assert(std::is_standard_layout_v<MeshFileHeader> == true);

#pragma pack(pop)

static size_t serialize(Serializer& serializer, const MeshFileHeader& mesh_header, size_t alignment = 1)
{
	const size_t begin = serialize(serializer, mesh_header.pos);
	serialize(serializer, mesh_header.nrm);
	serialize(serializer, mesh_header.uvs);
	serialize(serializer, mesh_header.colors);
	serialize(serializer, mesh_header.strips);
	serialize(serializer, mesh_header.prim_type);

	return begin;
}