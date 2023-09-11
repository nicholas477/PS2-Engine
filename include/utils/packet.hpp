#pragma once

#include <packet2.h>
#include <packet2_utils.h>

#include <memory>
#include <stdio.h>
#include <dma.h>

struct packet2_deleter
{
	void operator()(packet2_t* packet) const
	{
		//printf("destroying packet!\n");
		packet2_free(packet);
	}
};

/// @brief unique_ptr wrapper around packet2_t. Automatically calls `packet2_free` on destruction
class packet2: public std::unique_ptr<packet2_t, struct packet2_deleter>
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

	/// @brief Calls packet2_update with this packet and supplied function f.
	/// For example `update(draw_setup_environment, 0, frame, z)` turns into `packet2_update(this, draw_setup_environment(this->next, 0, frame, z))`
	/// @param f Function to call with packet2_update
	/// @param args args to pass to function `f`
	template <typename F, typename... Args>
	void update(F f, Args&&... args)
	{
		packet2_update(get(), f(get()->next, std::forward<Args>(args)...));
	}

	void add(float val)
	{
		packet2_add_float(*this, val);
	}

	void add(u32 val)
	{
		packet2_add_u32(*this, val);
	}

	void add(u64 val)
	{
		packet2_add_u64(*this, val);
	}

	void add(s32 val)
	{
		packet2_add_s32(*this, val);
	}

	void add(s64 val)
	{
		packet2_add_s64(*this, val);
	}

	void send(int channel, bool flush_cache)
	{
		dma_channel_send_packet2(*this, channel, flush_cache);
	}

	void add_end_tag()
	{
		packet2_utils_vu_add_end_tag(*this);
	}

	u32 get_qw_count() const
	{
		return packet2_get_qw_count(const_cast<packet2_t*>(get()));
	}

	operator const packet2_t*() const { return get(); }
	operator packet2_t*() { return get(); }
};