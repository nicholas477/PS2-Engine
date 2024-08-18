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
	// template definitions
	void initialize(enum Packet2Type type, enum Packet2Mode mode, u8 tte, u32 qwords, qword_t* data)
	{
		// Copied from packet2.c
		packet.max_qwords_count   = qwords;
		packet.base               = data;
		packet.next               = data;
		packet.type               = type;
		packet.mode               = mode;
		packet.tte                = tte;
		packet.tag_opened_at      = nullptr;
		packet.vif_code_opened_at = nullptr;

		// Dma buffer size should be a whole number of cache lines (64 bytes = 4 quads)
		assert(!((packet.type == P2_TYPE_UNCACHED || packet.type == P2_TYPE_UNCACHED_ACCL) && packet.max_qwords_count & (4 - 1)));

		u32 byte_size = qwords << 4;
		memset(packet.base, 0, byte_size);

		// "I hate to do this, but I've wasted FAR too much time hunting down cache incoherency"
		if (packet.type == P2_TYPE_UNCACHED || packet.type == P2_TYPE_UNCACHED_ACCL)
		{
			FlushCache(0);
		}
	}
};

// packet2 with memory allocated inline
template <size_t qwords>
struct inline_packet2: public inline_packet2_base
{
	qword_t data[qwords] __attribute__((aligned(16)));

	inline_packet2() = default;

	inline_packet2(enum Packet2Type type, enum Packet2Mode mode, u8 tte)
	{
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