#pragma once

#include <packet2.h>
#include <packet2_utils.h>

#include "assert.hpp"

#include <memory>
#include <stdio.h>
#include <dma.h>
#include <kernel.h>
#include <string.h>

/// @brief Static interface for packets. Meant to be a drop-in replacement for packet2_t*.
/// @tparam T Put your derived class here.
template <typename T>
class packet2_interface
{
public:
	/// @brief Calls packet2_update with this packet and supplied function f.
	/// For example `update(draw_setup_environment, 0, frame, z)` turns into `packet2_update(this, draw_setup_environment(this->next, 0, frame, z))`
	/// @param f Function to call with packet2_update
	/// @param args args to pass to function `f`
	template <typename F, typename... Args>
	void update(F f, Args&&... args)
	{
		packet2_update(to_ptr(), f(to_ptr()->next, std::forward<Args>(args)...));
	}

	void add(float val)
	{
		packet2_add_float(to_ptr(), val);
	}

	void add(u32 val)
	{
		packet2_add_u32(to_ptr(), val);
	}

	void add(u64 val)
	{
		packet2_add_u64(to_ptr(), val);
	}

	void add(s32 val)
	{
		packet2_add_s32(to_ptr(), val);
	}

	void add(s64 val)
	{
		packet2_add_s64(to_ptr(), val);
	}

	void send(int channel, bool flush_cache)
	{
		dma_channel_send_packet2(to_ptr(), channel, flush_cache);
	}

	void add_end_tag()
	{
		packet2_utils_vu_add_end_tag(to_ptr());
	}

	u32 get_qw_count() const
	{
		return packet2_get_qw_count(const_cast<packet2_t*>(to_const_ptr()));
	}


	const packet2_t* operator->() const { return to_const_ptr(); }
	packet2_t* operator->() { return to_ptr(); }

	const packet2_t* to_const_ptr() const { return static_cast<const T&>(*this); }
	packet2_t* to_ptr() { return static_cast<T&>(*this); }

	operator const packet2_t*() const { return static_cast<const T&>(*this); }
	operator packet2_t*() { return static_cast<T&>(*this); }
};


struct packet2_deleter
{
	void operator()(packet2_t* packet) const
	{
		//printf("destroying packet!\n");
		packet2_free(packet);
	}
};

/// @brief unique_ptr wrapper around packet2_t. Automatically calls `packet2_free` on destruction
class packet2: public std::unique_ptr<packet2_t, struct packet2_deleter>, public packet2_interface<packet2>
{
public:
	packet2() = default;

	packet2(packet2_t* packet)
	    : std::unique_ptr<packet2_t, struct packet2_deleter>(packet)
	{
	}

	/// @brief Creates a new packet using packet2_create
	/// @param qwordsMaximum data size in qwords (128bit).
	/// @param type Memory mapping type.
	/// @param mode Packet mode. Normal or chain.
	/// @param tte Tag transfer enable.
	/// Used only for CHAIN mode!
	/// If true, then transfer tag is set during pack sending and
	/// add_dma_tag() (so also every open_tag()) will move buffer by DWORD
	packet2(u16 qwords, enum Packet2Type type, enum Packet2Mode mode, bool tte)
	    : packet2(packet2_create(qwords, type, mode, tte))
	{
	}

	operator const packet2_t*() const { return get(); }
	operator packet2_t*() { return get(); }
};

/// @brief wrapper type for packet2_t* that allocates memory inline
template <u16 qwords>
class packet2_inline: public packet2_interface<packet2_inline<qwords>>
{
public:
	packet2_inline() = default;

	/// @brief Creates a new packet using packet2_create. Allocates memory inline
	/// @param qwordsMaximum data size in qwords (128bit).
	/// @param type Memory mapping type.
	/// @param mode Packet mode. Normal or chain.
	/// @param tte Tag transfer enable.
	/// Used only for CHAIN mode!
	/// If true, then transfer tag is set during pack sending and
	/// add_dma_tag() (so also every open_tag()) will move buffer by DWORD
	packet2_inline(enum Packet2Type type, enum Packet2Mode mode, bool tte)
	{
		memset(&packet, 0, sizeof(packet));
		packet.max_qwords_count   = qwords;
		packet.type               = type;
		packet.mode               = mode;
		packet.tte                = tte;
		packet.tag_opened_at      = NULL;
		packet.vif_code_opened_at = NULL;

		// Dma buffer size should be a whole number of cache lines (64 bytes = 4 quads)
		assert(!((packet.type == P2_TYPE_UNCACHED || packet.type == P2_TYPE_UNCACHED_ACCL) && packet.max_qwords_count & (4 - 1)));

		packet.base = (qword_t*)data.data();
		packet.base = packet.next = (qword_t*)((u32)packet.base | packet.type);

		memset(packet.base, 0, byte_size);

		// "I hate to do this, but I've wasted FAR too much time hunting down cache incoherency"
		if (packet.type == P2_TYPE_UNCACHED || packet.type == P2_TYPE_UNCACHED_ACCL)
			FlushCache(0);
	}

	static constexpr std::size_t byte_size = qwords << 4;
	packet2_t packet;
	alignas(64) std::array<u8, byte_size> data;

	operator const packet2_t*() const
	{
		return &packet;
	}
	operator packet2_t*() { return &packet; }

	packet2_inline(const packet2_inline&) = delete;
	packet2_inline& operator=(const packet2_inline&) = delete;

	//packet2_inline& operator=(packet2_inline&&) = default;
};