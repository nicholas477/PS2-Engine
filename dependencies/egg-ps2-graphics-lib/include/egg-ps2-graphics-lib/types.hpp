#pragma once

#include <stddef.h>
#include <packet2_types.h>
#include <cassert>
#include <cstring>
#include <kernel.h>

namespace egg::ps2::graphics::utils
{
struct inline_packet2_base
{
	packet2_t packet;

protected:
	inline_packet2_base() = default;

	// Initialize is defined in the base here so that it doesn't have a billion
	// identical template definitions
	void initialize(enum Packet2Type type, enum Packet2Mode mode, u8 tte, u32 qwords, qword_t* data);
};

// packet2 with memory allocated inline
template <size_t qwords>
struct inline_packet2: public inline_packet2_base
{
	qword_t data[qwords] __attribute__((aligned(16)));

	inline_packet2()
	{
		assert(__is_aligned(&data[0], 16));
	};

	inline_packet2(enum Packet2Type type, enum Packet2Mode mode, u8 tte)
	{
		assert(__is_aligned(&data[0], 16));

		initialize(type, mode, tte);
	}

	void initialize(enum Packet2Type type, enum Packet2Mode mode, u8 tte)
	{
		inline_packet2_base::initialize(type, mode, tte, qwords, data);
	}

	operator packet2_t*() { return &packet; }
	operator const packet2_t*() const { return &packet; }

	packet2_t* operator->() { return &packet; }
	const packet2_t* operator->() const { return &packet; }
};
} // namespace egg::ps2::graphics::utils