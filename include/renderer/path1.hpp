// Taken from Tyra game engine
// https://github.com/h4570/tyra/blob/master/engine/inc/renderer/core/paths/path1/path1.hpp

#pragma once


#include "utils/packet.hpp"

#include <packet2_utils.h>
#include <tamtypes.h>

class VU1Program;

class Path1
{
public:
	Path1();

	void uploadProgram(VU1Program& program);

	void sendDrawFinishTag();
	void addDrawFinishTag(packet2_t* packet);

	void setDoubleBuffer(const u16& startingAddress, const u16& bufferSize);

private:
	void uploadDrawFinishProgram();
	void prepareDrawFinishPacket();

	packet2_inline<2> doubleBufferPacket;
	packet2_inline<10> drawFinishPacket;
	u32 currentProgramAddress;
};