#pragma once

#include <assert.h>
#include <cstdint>
#include <type_traits>

#if __has_include(<span>)
#include <span>
#endif

#include "egg/serialization.hpp"

template <typename T>
struct OffsetPointer
{
	// Offset (in bytes)
	// This is generally an offset from the first byte of THIS OffsetPointer
	int32_t offset;

	T* get_ptr() { return reinterpret_cast<T*>(((uint8_t*)this) + offset); }

	const T* get_ptr() const
	{
		return reinterpret_cast<const T*>(((const uint8_t*)this) + offset);
	}

	using elem_T = T;
};

template <typename T>
struct OffsetArray
{
	// Offset (in bytes)
	// This is generally an offset from the first byte of THIS OffsetPointer
	union
	{
#if INTPTR_MAX == INT32_MAX
		int32_t offset;
		int64_t offset_64;
#else
		ptrdiff_t offset;
#endif
	};

	// Length of the array (in bytes)
	int32_t length;

	T* get_ptr() { return reinterpret_cast<T*>(((uint8_t*)this) + offset); }

	const T* get_ptr() const
	{
		return reinterpret_cast<const T*>(((const uint8_t*)this) + offset);
	}

	T* get_array() { return this->get_ptr(); }

	const T* get_array() const { return this->get_ptr(); }

	T& operator[](int32_t index)
	{
		assert(index >= 0 && index < num_elements());
		return get_array()[index];
	}

	const T& operator[](int32_t index) const
	{
		assert(index >= 0 && index < num_elements());
		return get_array()[index];
	}

	T* begin() { return get_array(); }

	const T* begin() const { return get_array(); }

	T* end() { return &get_array()[num_elements()]; }

	const T* end() const { return &get_array()[num_elements()]; }

	void set(intptr_t data, size_t in_length)
	{
		ptrdiff_t new_offset = data - (intptr_t)this;
		offset               = new_offset;
		assert(offset == new_offset);
		length = in_length;
		assert(length == in_length);
	}

	// Points this offset pointer to this array
	// Does not take ownership of the array's data!
	template <typename vecT>
	void set(const std::vector<vecT>& vec)
	{
		set((intptr_t)vec.data(), vec.size() * sizeof(vecT));
	}

#ifdef __cpp_lib_span
	std::span<T> get_span()
	{
		return std::span<T>(get_array(), num_elements());
	}
#endif

	int32_t num_elements() const
	{
		return length / sizeof(T);
	}

	void set_num_elements(int32_t num_elements)
	{
		length = num_elements * sizeof(T);
	}
};

template <typename T>
static size_t serialize(Serializer& serializer, const OffsetArray<T>& val, size_t alignment = 1)
{
	const size_t offset_array_start = serializer.add_data(val, alignment);

	class OffsetArrayHeapSerializer: public HeapSerializer
	{
	public:
		OffsetArrayHeapSerializer(size_t in_offset_array_start,
		                          const OffsetArray<T>& in_arr)
		    : offset_array_start(in_offset_array_start)
		    , arr(in_arr)
		{
		}

		size_t offset_array_start;
		const OffsetArray<T>& arr;

		virtual void heap_serialize(Serializer& serializer) override
		{
			size_t data_index = serializer.align_current_byte_to(alignof(T));
			for (const T& val : arr)
			{
				serialize(serializer, val, 1);
			}

			OffsetArray<T>* offset_array = (OffsetArray<T>*)&serializer.archive->at(offset_array_start);
			offset_array->offset         = data_index - offset_array_start;
			offset_array->length         = arr.length;

			assert(memcmp(arr.get_ptr(), offset_array->get_ptr(), offset_array->length) == 0);
		}
	};

	serializer.heap_data_serializer.push(new OffsetArrayHeapSerializer(offset_array_start, val));
	return offset_array_start;
}