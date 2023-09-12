#pragma once

#include <packet2.h>
#include <packet2_utils.h>

#include <memory>
#include <stdio.h>
#include <dma.h>
#include <string.h>

static constexpr std::size_t packet_data_alignment = 64;

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

	void reset(bool clear_mem)
	{
		packet2_reset(to_ptr(), clear_mem);
	}

	///////// Override THESE functions
	const packet2_t* to_const_ptr() const { return static_cast<const T&>(*this).to_const_ptr(); }
	packet2_t* to_ptr() { return static_cast<T&>(*this).to_ptr(); }
	/////////

	const packet2_t* operator->() const { return to_const_ptr(); }
	packet2_t* operator->() { return to_ptr(); }

	operator const packet2_t*() const { return to_const_ptr(); }
	operator packet2_t*() { return to_ptr(); }
};

/// @brief Heap allocated wrapper around packet2_t
class packet2: public packet2_interface<packet2>
{
public:
	packet2() = default;

	/// @brief Creates a new packet using packet2_create
	/// @param qwordsMaximum data size in qwords (128bit).
	/// @param type Memory mapping type.
	/// @param mode Packet mode. Normal or chain.
	/// @param tte Tag transfer enable.
	/// Used only for CHAIN mode!
	/// If true, then transfer tag is set during pack sending and
	/// add_dma_tag() (so also every open_tag()) will move buffer by DWORD
	packet2(u16 qwords, enum Packet2Type type, enum Packet2Mode mode, bool tte);

	const packet2_t* to_const_ptr() const { return &packet; }
	packet2_t* to_ptr() { return &packet; }

	alignas(packet_data_alignment) packet2_t packet;
	std::unique_ptr<u8> data;

	packet2(const packet2&) = delete;
	packet2& operator=(const packet2&) = delete;

	packet2& operator=(packet2&& other)
	{
		packet = std::move(other.packet);
		data   = std::move(other.data);
		printf("Moving packet.\n");
		return *this;
	}
};

void initialize_packet2_inline(packet2_t& packet, qword_t* data, u16 qwords, enum Packet2Type type, enum Packet2Mode mode, bool tte);

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
		initialize_packet2_inline(packet, (qword_t*)data.data(), qwords, type, mode, tte);
	}

	static constexpr std::size_t byte_size = qwords << 4;
	alignas(packet_data_alignment) packet2_t packet;
	alignas(packet_data_alignment) std::array<u8, byte_size> data;

	const packet2_t* to_const_ptr() const { return &packet; }
	packet2_t* to_ptr() { return &packet; }

	packet2_inline(const packet2_inline&) = delete;
	packet2_inline& operator=(const packet2_inline&) = delete;

	packet2_inline& operator=(packet2_inline&&) = default;
};