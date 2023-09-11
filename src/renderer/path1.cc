// Taken from Tyra game engine
// https://github.com/h4570/tyra/blob/master/engine/src/renderer/core/paths/path1/path1.cpp

#include "renderer/path1.hpp"
#include "renderer/vu1_program.hpp"

#include "renderer/vu1programs/draw_finish.hpp"
#include "renderer/vu1programs/draw_3D.hpp"
#include "renderer/vu1programs/color_triangles_clip_tris.hpp"
#include "renderer/vu1programs/color_triangles_clip_verts.hpp"

#include <dma.h>

Path1::Path1()
{
	currentProgramAddress = 0;
	doubleBufferPacket    = packet2_create(2, P2_TYPE_NORMAL, P2_MODE_CHAIN, true);
	drawFinishPacket      = packet2_create(10, P2_TYPE_NORMAL, P2_MODE_CHAIN, true);

	uploadProgram(draw_finish::get());
	prepareDrawFinishPacket();

	uploadProgram(draw_3D::get());
	//uploadProgram(color_triangles_clip_tris::get());

	setDoubleBuffer(8, 496); // No idea how these numbers are picked.
}

void Path1::uploadProgram(VU1Program& program)
{
	printf("uploading vu1 program: %s\n", program.getStringName().c_str());
	u32 packetSize = program.getPacketSize() + 1;

	packet2 packet =
	    packet2_create(packetSize, P2_TYPE_NORMAL, P2_MODE_CHAIN, 1);

	program.setDestinationAddress(currentProgramAddress);
	packet2_vif_add_micro_program(packet, currentProgramAddress, program.getStart(),
	                              program.getEnd());
	currentProgramAddress += program.getProgramSize() + 1;

	packet2_utils_vu_add_end_tag(packet);

	// Actually upload the program now
	dma_channel_wait(DMA_CHANNEL_VIF1, 0);
	packet.send(DMA_CHANNEL_VIF1, true);
	dma_channel_wait(DMA_CHANNEL_VIF1, 0);
}

void Path1::addDrawFinishTag(packet2_t* packet)
{
	prim_t prim;
	prim.type         = PRIM_TRIANGLE;
	prim.shading      = PRIM_SHADE_GOURAUD;
	prim.mapping      = 1;
	prim.fogging      = 0;
	prim.blending     = 1;
	prim.antialiasing = 0;
	prim.mapping_type = PRIM_MAP_ST;
	prim.colorfix     = PRIM_UNFIXED;

	packet2_utils_vu_open_unpack(packet, 10, true);
	{
		packet2_utils_gif_add_set(packet, 1);
		packet2_utils_gs_add_draw_finish_giftag(packet);
		packet2_utils_gs_add_prim_giftag(packet, &prim, 0,
		                                 ((u64)GIF_REG_RGBAQ) << 0, 1, 0);
	}
	packet2_utils_vu_close_unpack(packet);

	packet2_utils_vu_add_start_program(packet, draw_finish::get().getDestinationAddress());
}

void Path1::sendDrawFinishTag()
{
	dma_channel_wait(DMA_CHANNEL_VIF1, 0);
	dma_channel_send_packet2(drawFinishPacket, DMA_CHANNEL_VIF1, true);
}

void Path1::prepareDrawFinishPacket()
{
	addDrawFinishTag(drawFinishPacket);
	packet2_utils_vu_add_end_tag(drawFinishPacket);
}

/** Set double buffer settings */
void Path1::setDoubleBuffer(const u16& startingAddress, const u16& bufferSize)
{
	packet2_reset(doubleBufferPacket, false);
	packet2_utils_vu_add_double_buffer(doubleBufferPacket, startingAddress,
	                                   bufferSize);


	doubleBufferPacket.add_end_tag();
	doubleBufferPacket.send(DMA_CHANNEL_VIF1, true);
}