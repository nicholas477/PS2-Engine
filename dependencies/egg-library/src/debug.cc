#include "egg/debug.hpp"

#include "egg/assert.hpp"
#include "elf.h"
#include <cxxabi.h>

namespace Debug
{
static int _argc    = 0;
static char** _argv = nullptr;

void init(int argc, char** argv)
{
	_argc = argc;
	_argv = argv;
}

static FILE* f = nullptr;

static const char* lookup_symbol(const Elf32_Sym& symbol, const Elf32_Shdr& string_section_header)
{
	if (symbol.st_name == 0)
	{
		printf("Error! symbol has no name!\n");
		return nullptr;
	}

	if (string_section_header.sh_type != SHT_STRTAB)
	{
		printf("Error! Tried to lookup symbol from non-string table!\n");
		return nullptr;
	}

	fseek(f, string_section_header.sh_offset + symbol.st_name, SEEK_SET);

	static char buff[1024];
	fgets(buff, 1024, f);

	int status                 = -1;
	const char* demangled_name = abi::__cxa_demangle(buff, NULL, NULL, &status);
	if (status == 0)
	{
		return demangled_name;
	}
	else
	{
		return buff;
	}
}

SymbolInfo lookup_symbol(void* symbol_addr)
{
	static bool bRanAlready = false;

	if (bRanAlready)
	{
		return nullptr;
	}

	if (_argc == 0)
	{
		return nullptr;
	}

	if (_argv == 0)
	{
		return nullptr;
	}

	if (f == nullptr)
	{
		f = fopen(_argv[0], "r");
	}

	if (f == nullptr)
	{
		return nullptr;
	}

	fseek(f, 0, SEEK_SET);

	Elf32_Ehdr file_header;

	static_assert(offsetof(Elf32_Ehdr, e_shoff) == 0x20);
	static_assert(offsetof(Elf32_Shdr, sh_offset) == 0x10);

	fread((char*)&file_header, sizeof(file_header), 1, f);

	fseek(f, file_header.e_shoff, SEEK_SET);

	// printf("Num sections: %d\n", file_header.e_shnum);
	// printf("String section num: %d\n", file_header.e_shstrndx);

	if (file_header.e_shentsize != sizeof(Elf32_Shdr))
	{
		// printf("elf section header size mismatch!!!\n");
		// printf("shentsize: %d\n", file_header.e_shentsize);
		// printf("sizeof(elf_section_header): %d\n", sizeof(elf_section_header));

		return nullptr;
		//exit(-1);
	}

	int32_t symbol_table_count = 0;
	int32_t string_table_count = 0;
	Elf32_Sym closest_address  = {};

	for (int i = 0; i < file_header.e_shnum; ++i)
	{
		Elf32_Shdr section_header;
		fread((char*)&section_header, sizeof(section_header), 1, f);

		if (section_header.sh_type == SHT_SYMTAB)
		{
			symbol_table_count++;
			//printf("Found symbol table!!!!!!!!!!!!!!!!!!!\n");

#ifdef _EE
			//printf("--------------------------------------------------------\n");
			long current_file_loc = ftell(f);
			if (section_header.sh_entsize != sizeof(Elf32_Sym))
			{
				//printf("symbol size mismatch!!!\n");
				return nullptr;
			}

			const int num_symbols = (section_header.sh_size / section_header.sh_entsize);
			//printf("Num symbols: %d\n", num_symbols);

			bool bFoundSymbol = false;
			fseek(f, section_header.sh_offset, SEEK_SET);
			for (int j = 0; j < num_symbols; ++j)
			{
				Elf32_Sym sym;
				fread((char*)&sym, sizeof(sym), 1, f);

				if (sym.st_value <= (Elf32_Addr)symbol_addr)
				{
					if (((Elf32_Addr)symbol_addr - closest_address.st_value) > ((Elf32_Addr)symbol_addr - sym.st_value))
					{
						closest_address = sym;
					}
				}
			}

			// printf("Symbol: 0x%x, Closest symbol: 0x%x\n", (int64_t)symbol_addr, closest_address.st_value);
			// printf("Difference: 0x%x\n", (int64_t)symbol_addr - closest_address.st_value);
			// printf("--------------------------------------------------------\n");

			fseek(f, current_file_loc, SEEK_SET);
#endif
		}

		if (i == file_header.e_shstrndx - 1)
		{
			return SymbolInfo((uint32_t)closest_address.st_value, lookup_symbol(closest_address, section_header));
		}
	}

	return nullptr;
}

} // namespace Debug
