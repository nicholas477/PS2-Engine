#pragma once

#include <cstdint>
#include <type_traits>
#include <assert.h>

#if __has_include(<span>)
#include <span>
#endif

template <typename T, typename size_type_T = uint32_t>
struct OffsetPointer
{
	// Offset (in bytes)
	// This is generally an offset from the first byte of THIS OffsetPointer
	size_type_T offset;

	T* get_ptr()
	{
		return reinterpret_cast<T*>(((uint8_t*)this) + offset);
	}

	const T* get_ptr() const
	{
		return reinterpret_cast<const T*>(((const uint8_t*)this) + offset);
	}

	using elem_T = T;
	static_assert(std::is_arithmetic_v<size_type_T> == true);
};

template <typename T, typename size_type_T = uint32_t>
struct OffsetArray: public OffsetPointer<T, size_type_T>
{
	// Length of the array (in bytes)
	size_type_T length;

	T* get_array()
	{
		return this->get_ptr();
	}

	const T* get_array() const
	{
		return this->get_ptr();
	}

	T& operator[](size_t index)
	{
		assert(index >= 0 && index < num_elements());
		return get_array()[index];
	}

	const T& operator[](size_t index) const
	{
		assert(index >= 0 && index < num_elements());
		return get_array()[index];
	}

	T* begin()
	{
		return get_array();
	}

	const T* begin() const
	{
		return get_array();
	}

	T* end()
	{
		return &get_array()[num_elements()];
	}

	const T* end() const
	{
		return &get_array()[num_elements()];
	}


#ifdef __cpp_lib_span
	std::span<T> get_span()
	{
		return std::span<T>(get_array(), num_elements());
	}
#endif

	size_type_T num_elements() const
	{
		return length / sizeof(T);
	}

	void set_num_elements(size_type_T num_elements)
	{
		length = num_elements * sizeof(T);
	}
};