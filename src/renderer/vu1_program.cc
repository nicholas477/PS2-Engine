// Taken from Tyra game engine
// https://github.com/h4570/tyra/blob/master/engine/src/renderer/core/paths/path1/vu1_program.cpp

#include "renderer/vu1_program.hpp"
#include "assert.hpp"
#include <packet2_utils.h>

VU1Program::VU1Program(u32* t_start, u32* t_end)
    : start(t_start)
    , end(t_end)
{
	check(t_start != 0);
	check(t_end != 0);

	// =1 means uninitialized
	destinationAddress = -1;
	packetSize         = packet2_utils_get_packet_size_for_program(start, end);
	programSize        = calculateProgramSize();

	check(programSize != 0);
}

u32 VU1Program::getPacketSize() const { return packetSize; }
u32 VU1Program::getProgramSize() const { return programSize; }
u32 VU1Program::getDestinationAddress() const
{
	check(destinationAddress != -1);
	return destinationAddress;
}
void VU1Program::setDestinationAddress(const u32& addr)
{
	check(destinationAddress == -1);
	destinationAddress = addr;
}
u32* VU1Program::getStart() const { return start; }
u32* VU1Program::getEnd() const { return end; }

u32 VU1Program::calculateProgramSize() const
{
	u32 count = (getEnd() - getStart()) / 2;
	if (count & 1)
		count++;

	return count;
}