#pragma once

#include "offset_pointer.hpp"
#include "serialization.hpp"

#pragma pack(push, 1)

struct TextureFileHeader
{
	uint16_t size_x;
	uint16_t size_y;

	OffsetArray<uint32_t> clut;
	OffsetArray<std::byte> data;
};

#pragma pack(pop)

static size_t serialize(Serializer& serializer, const TextureFileHeader& texture_header, size_t alignment = 1)
{
	const size_t begin = serialize(serializer, texture_header.size_x);
	serialize(serializer, texture_header.size_y);
	serialize(serializer, texture_header.clut, 1, 16);
	serialize(serializer, texture_header.data, 1, 16);
	return begin;
}