#pragma once

#include "offset_pointer.hpp"
#include "serialization.hpp"

#pragma pack(push, 1)

struct TextureFileHeader
{
	uint16_t size_x;
	uint16_t size_y;

	uint16_t psm;
	uint8_t function;
	uint8_t components;

	// Wrapping settings
	uint8_t horizontal;
	uint8_t vertical;

	int32_t minu;
	int32_t maxu;
	int32_t minv;
	int32_t maxv;

	OffsetArray<uint32_t> clut;
	OffsetArray<std::byte> data;
};

#pragma pack(pop)

static size_t serialize(Serializer& serializer, const TextureFileHeader& texture_header, size_t alignment = 1)
{
	const size_t begin = serialize(serializer, texture_header.size_x);
	serialize(serializer, texture_header.size_y);
	serialize(serializer, texture_header.psm);
	serialize(serializer, texture_header.function);
	serialize(serializer, texture_header.components);

	serialize(serializer, texture_header.horizontal);
	serialize(serializer, texture_header.vertical);

	serialize(serializer, texture_header.minu);
	serialize(serializer, texture_header.maxu);
	serialize(serializer, texture_header.minv);
	serialize(serializer, texture_header.maxv);

	serialize(serializer, texture_header.clut, 1, 16);
	serialize(serializer, texture_header.data, 1, 16);
	return begin;
}