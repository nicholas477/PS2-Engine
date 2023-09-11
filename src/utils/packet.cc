#include "utils/packet.hpp"

#include <kernel.h>
#include <string.h>

void initialize_packet2_inline(packet2_t& packet, qword_t* data, u16 qwords, enum Packet2Type type, enum Packet2Mode mode, bool tte)
{
	//memset(&packet, 0, sizeof(packet));

	packet.max_qwords_count   = qwords;
	packet.type               = type;
	packet.mode               = mode;
	packet.tte                = tte;
	packet.tag_opened_at      = NULL;
	packet.vif_code_opened_at = NULL;

	// Dma buffer size should be a whole number of cache lines (64 bytes = 4 quads)
	assert(!((packet.type == P2_TYPE_UNCACHED || packet.type == P2_TYPE_UNCACHED_ACCL) && packet.max_qwords_count & (4 - 1)));

	packet.base = data;
	packet.base = packet.next = (qword_t*)((u32)packet.base | packet.type);

	// "I hate to do this, but I've wasted FAR too much time hunting down cache incoherency"
	if (packet.type == P2_TYPE_UNCACHED || packet.type == P2_TYPE_UNCACHED_ACCL)
		FlushCache(0);
}