#pragma once

#include <cstddef>
#include <queue>
#include <stdio.h>
#include <string.h>
#include <vector>

// Pads a number to the next aligned number
// pad_to_alignment(3, 8) = 8
static constexpr size_t pad_to_alignment(size_t current_index,
                                         size_t alignment = 16)
{
	ptrdiff_t mask = alignment - 1;
	return current_index + (-current_index & mask);
}

static_assert(pad_to_alignment(3, 8) == 8);
static_assert(pad_to_alignment(9, 8) == 16);
static_assert(pad_to_alignment(0, 8) == 0);
static_assert(pad_to_alignment(1, 1) == 1);
static_assert(pad_to_alignment(6, 1) == 6);

class HeapSerializer
{
public:
	virtual void heap_serialize(class Serializer& serializer) {}

	virtual ~HeapSerializer() {};
};

class Serializer
{
public:
	Serializer()
	{
		archive      = nullptr;
		current_byte = 0;
	}

	Serializer(std::vector<std::byte>& in_archive)
	{
		current_byte = 0;

		in_archive.resize(0);
		archive = &in_archive;
	}

	virtual ~Serializer()
	{
		finish_serialization();
	}

	virtual void finish_serialization()
	{
		while (!heap_data_serializer.empty())
		{
			HeapSerializer* heap_serializer = heap_data_serializer.front();
			heap_serializer->heap_serialize(*this);
			delete heap_serializer;

			heap_data_serializer.pop();
		}
	}

	std::vector<std::byte>* archive;

	size_t current_byte;

	std::queue<HeapSerializer*> heap_data_serializer;

	// Serializes the data into the archive. Pads the start of the data to
	// alignment Returns the index in the archive of the newly added data
	template <typename T>
	size_t add_data(const T& data, size_t alignment = 1)
	{
		return add_data((const void*)&data, sizeof(data), alignment);
	}

	// Serializes the data into the archive. Pads the start of the data to
	// alignment Returns the index in the archive of the newly added data
	size_t add_data(const void* data, size_t data_size, size_t alignment = 1)
	{
		current_byte = pad_to_alignment(current_byte, alignment);
		archive->resize(data_size + current_byte);

		memcpy((void*)&(archive->at(current_byte)), data, data_size);

		const size_t out_byte = current_byte;
		current_byte          = archive->size();

		return out_byte;
	}
};

static size_t serialize(Serializer& serializer, const float& val)
{
	return serializer.add_data(val);
}