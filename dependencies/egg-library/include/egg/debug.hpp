#pragma once

#include <cstdint>

namespace Debug
{
struct SymbolInfo
{
	SymbolInfo() = default;
	SymbolInfo(uint32_t in_address, const char* in_name)
	    : address(in_address)
	    , name(in_name)
	{
	}
	SymbolInfo(decltype(nullptr)) {};

	uint32_t address = 0;
	const char* name = nullptr;

	operator bool() const
	{
		return address != 0 && name != nullptr;
	}
};

void init(int argc, char** argv);
SymbolInfo lookup_symbol(void* symbol_addr);
} // namespace Debug