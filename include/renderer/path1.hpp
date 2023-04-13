// Taken from Tyra game engine
// https://github.com/h4570/tyra/blob/master/engine/inc/renderer/core/paths/path1/path1.hpp

#pragma once

#include <packet2_utils.h>
#include <tamtypes.h>

class VU1Program;

class Path1
{
public:
	Path1();
	~Path1();

	u32 uploadProgram(VU1Program* program, const u32& address);

	void sendDrawFinishTag();

	void addDrawFinishTag(packet2_t* packet);

	packet2_t* createProgramsCache(VU1Program** programs, const u32& count,
	                               const u32& address);

	void setDoubleBuffer(const u16& startingAddress, const u16& bufferSize);

private:
	void uploadDrawFinishProgram();
	void prepareDrawFinishPacket();

	packet2_t* doubleBufferPacket;
	packet2_t* drawFinishPacket;
	u32 drawFinishAddr;
};