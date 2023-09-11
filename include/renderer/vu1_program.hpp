// Taken from Tyra game engine
// https://github.com/h4570/tyra/blob/master/engine/inc/renderer/core/paths/path1/vu1_program.hpp

#pragma once

#include <tamtypes.h>
#include <string>

class VU1Program
{
public:
	VU1Program(u32* start, u32* end);

	u32 getPacketSize() const;
	u32 getProgramSize() const;
	u32 getDestinationAddress() const;

	u32* getStart() const;
	u32* getEnd() const;

	virtual std::string getStringName() const = 0;

	void setDestinationAddress(const u32& addr);

protected:
	u32 packetSize, destinationAddress, programSize;
	u32 *start, *end;
	u32 calculateProgramSize() const;
};