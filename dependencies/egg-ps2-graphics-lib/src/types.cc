#include "egg-ps2-graphics-lib/types.hpp"

namespace egg::ps2::graphics::utils
{
void inline_packet2_base::initialize(enum Packet2Type type, enum Packet2Mode mode, u8 tte, u32 qwords, qword_t* data)
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
} // namespace egg::ps2::graphics::utils