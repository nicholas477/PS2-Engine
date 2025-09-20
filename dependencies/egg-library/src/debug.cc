#include "egg/debug.hpp"

#include "egg/assert.hpp"
#include "elf.h"
#include <cxxabi.h>

namespace Debug
{
static int _argc    = 0;
static char** _argv = nullptr;

struct __attribute__((__packed__)) elf_header
{
	unsigned char e_ident[EI_NIDENT];
	/**
	 * This member of the structure identifies the object file
       type:

              ET_NONE
                     An unknown type.
              ET_REL A relocatable file.
              ET_EXEC
                     An executable file.
              ET_DYN A shared object.
              ET_CORE
                     A core file.
	 */
	Elf32_Half e_type;

	/**
	 * This member specifies the required architecture for an
       individual file.  For example:

              EM_NONE
                     An unknown machine
              EM_M32 AT&T WE 32100
              EM_SPARC
                     Sun Microsystems SPARC
              EM_386 Intel 80386
              EM_68K Motorola 68000
              EM_88K Motorola 88000
              EM_860 Intel 80860
              EM_MIPS
                     MIPS RS3000 (big-endian only)
              EM_PARISC
                     HP/PA
              EM_SPARC32PLUS
                     SPARC with enhanced instruction set
              EM_PPC PowerPC
              EM_PPC64
                     PowerPC 64-bit
              EM_S390
                     IBM S/390
              EM_ARM Advanced RISC Machines
              EM_SH  Renesas SuperH
              EM_SPARCV9
                     SPARC v9 64-bit
              EM_IA_64
                     Intel Itanium
              EM_X86_64
                     AMD x86-64
              EM_VAX DEC Vax
	 */
	Elf32_Half e_machine;

	/**
	 * This member identifies the file version:

              EV_NONE
                     Invalid version
              EV_CURRENT
                     Current version
	 */
	Elf32_Word e_version;

	/**
	 * This member gives the virtual address to which the system
       first transfers control, thus starting the process.  If the
       file has no associated entry point, this member holds zero.
	 */
	Elf32_Addr e_entry;

	/**
	 * This member holds the program header table's file offset in
	   bytes.  If the file has no program header table, this
	   member holds zero.
	 */
	Elf32_Off e_phoff;

	/**
	 * This member holds the section header table's file offset in
       bytes.  If the file has no section header table, this
       member holds zero.
	 */
	Elf32_Off e_shoff;

	/**
	 * This member holds processor-specific flags associated with
	   the file.  Flag names take the form EF_`machine_flag'.
       Currently, no flags have been defined.
	 */
	Elf32_Word e_flags;

	/**
	 * This member holds the ELF header's size in bytes.
	 */
	Elf32_Half e_ehsize;

	/**
	 *  This member holds the size in bytes of one entry in the
		file's program header table; all entries are the same size.
	 */
	Elf32_Half e_phentsize;

	/**
	 * This member holds the number of entries in the program
       header table.  Thus the product of e_phentsize and e_phnum
       gives the table's size in bytes.  If a file has no program
       header, e_phnum holds the value zero.

       If the number of entries in the program header table is
       larger than or equal to PN_XNUM (0xffff), this member holds
       PN_XNUM (0xffff) and the real number of entries in the
       program header table is held in the sh_info member of the
       initial entry in section header table.  Otherwise, the
       sh_info member of the initial entry contains the value
       zero.

              PN_XNUM
                     This is defined as 0xffff, the largest number
                     e_phnum can have, specifying where the actual number
                     of program headers is assigned.
	 */
	Elf32_Half e_phnum;

	/**
	 * This member holds a sections header's size in bytes.  A
       section header is one entry in the section header table;
       all entries are the same size.
	 */
	Elf32_Half e_shentsize;

	/**
	 * This member holds the number of entries in the section
       header table.  Thus the product of e_shentsize and e_shnum
       gives the section header table's size in bytes.  If a file
       has no section header table, e_shnum holds the value of
       zero.

       If the number of entries in the section header table is
       larger than or equal to SHN_LORESERVE (0xff00), e_shnum
       holds the value zero and the real number of entries in the
       section header table is held in the sh_size member of the
       initial entry in section header table.  Otherwise, the
       sh_size member of the initial entry in the section header
       table holds the value zero.
	 */
	Elf32_Half e_shnum;

	/**
	 * This member holds the section header table index of the
       entry associated with the section name string table.  If
       the file has no section name string table, this member
       holds the value SHN_UNDEF.

       If the index of section name string table section is larger
       than or equal to SHN_LORESERVE (0xff00), this member holds
       SHN_XINDEX (0xffff) and the real index of the section name
       string table section is held in the sh_link member of the
       initial entry in section header table.  Otherwise, the
       sh_link member of the initial entry in section header table
       contains the value zero.
	 */
	Elf32_Half e_shstrndx;
};

struct __attribute__((packed)) elf_section_header
{
	/**
	 * This member specifies the name of the section.  Its value
       is an index into the section header string table section,
       giving the location of a null-terminated string.
	 */
	Elf32_Word sh_name;

	/**
	 * This member categorizes the section's contents and
       semantics.

              SHT_NULL
                     This value marks the section header as inactive.  It
                     does not have an associated section.  Other members
                     of the section header have undefined values.

              SHT_PROGBITS
                     This section holds information defined by the
                     program, whose format and meaning are determined
                     solely by the program.

              SHT_SYMTAB
                     This section holds a symbol table.  Typically,
                     SHT_SYMTAB provides symbols for link editing, though
                     it may also be used for dynamic linking.  As a
                     complete symbol table, it may contain many symbols
                     unnecessary for dynamic linking.  An object file can
                     also contain a SHT_DYNSYM section.  The index of the
                     associated string table section can be found in the
                     sh_link member.

              SHT_STRTAB
                     This section holds a string table.  An object file
                     may have multiple string table sections.

              SHT_RELA
                     This section holds relocation entries with explicit
                     addends, such as type Elf32_Rela for the 32-bit
                     class of object files.  An object may have multiple
                     relocation sections.

              SHT_HASH
                     This section holds a symbol hash table.  An object
                     participating in dynamic linking must contain a
                     symbol hash table.  An object file may have only one
                     hash table.

              SHT_DYNAMIC
                     This section holds information for dynamic linking.
                     An object file may have only one dynamic section.

              SHT_NOTE
                     This section holds notes (ElfN_Nhdr).

              SHT_NOBITS
                     A section of this type occupies no space in the file
                     but otherwise resembles SHT_PROGBITS.  Although this
                     section contains no bytes, the sh_offset member
                     contains the conceptual file offset.

              SHT_REL
                     This section holds relocation offsets without
                     explicit addends, such as type Elf32_Rel for the
                     32-bit class of object files.  An object file may
                     have multiple relocation sections.

              SHT_SHLIB
                     This section is reserved but has unspecified
                     semantics.

              SHT_DYNSYM
                     This section holds a minimal set of dynamic linking
                     symbols.  An object file can also contain a
                     SHT_SYMTAB section.

              SHT_LOPROC
              SHT_HIPROC
                     Values in the inclusive range [SHT_LOPROC,
                     SHT_HIPROC] are reserved for processor-specific
                     semantics.

              SHT_LOUSER
                     This value specifies the lower bound of the range of
                     indices reserved for application programs.

              SHT_HIUSER
                     This value specifies the upper bound of the range of
                     indices reserved for application programs.  Section
                     types between SHT_LOUSER and SHT_HIUSER may be used
                     by the application, without conflicting with current
                     or future system-defined section types.
	 */
	Elf32_Word sh_type;

	/**
	 * Sections support one-bit flags that describe miscellaneous
       attributes.  If a flag bit is set in sh_flags, the
       attribute is "on" for the section.  Otherwise, the
       attribute is "off" or does not apply.  Undefined attributes
       are set to zero.

              SHF_WRITE
                     This section contains data that should be writable
                     during process execution.

              SHF_ALLOC
                     This section occupies memory during process
                     execution.  Some control sections do not reside in
                     the memory image of an object file.  This attribute
                     is off for those sections.

              SHF_EXECINSTR
                     This section contains executable machine
                     instructions.

              SHF_MASKPROC
                     All bits included in this mask are reserved for
                     processor-specific semantics.
	 */
	Elf32_Word sh_flags;

	/**
	 * If this section appears in the memory image of a process,
       this member holds the address at which the section's first
       byte should reside.  Otherwise, the member contains zero.
	 */
	Elf32_Addr sh_addr;

	/**
	 * This member's value holds the byte offset from the
       beginning of the file to the first byte in the section.
       One section type, SHT_NOBITS, occupies no space in the
       file, and its sh_offset member locates the conceptual
       placement in the file.
	 */
	Elf32_Off sh_offset;

	/**
	 * This member holds the section's size in bytes.  Unless the
       section type is SHT_NOBITS, the section occupies sh_size
       bytes in the file.  A section of type SHT_NOBITS may have a
       nonzero size, but it occupies no space in the file.
	 */
	Elf32_Word sh_size;

	/**
	 * This member holds a section header table index link, whose
       interpretation depends on the section type.
	 */
	Elf32_Word sh_link;

	/**
	 * This member holds extra information, whose interpretation
       depends on the section type.
	 */
	Elf32_Word sh_info;

	/**
	 * Some sections have address alignment constraints.  If a
       section holds a doubleword, the system must ensure
       doubleword alignment for the entire section.  That is, the
       value of sh_addr must be congruent to zero, modulo the
       value of sh_addralign.  Only zero and positive integral
       powers of two are allowed.  The value 0 or 1 means that the
       section has no alignment constraints.
	 */
	Elf32_Word sh_addralign;

	/**
	 * Some sections hold a table of fixed-sized entries, such as
       a symbol table.  For such a section, this member gives the
       size in bytes for each entry.  This member contains zero if
       the section does not hold a table of fixed-size entries.
	 */
	Elf32_Word sh_entsize;
};

struct __attribute__((packed)) Elf32_Sym
{
	/**
	 * This member holds an index into the object file's symbol
       string table, which holds character representations of the
       symbol names.  If the value is nonzero, it represents a
       string table index that gives the symbol name.  Otherwise,
       the symbol has no name.
	 */
	uint32_t st_name;

	/**
	 * This member gives the value of the associated symbol.
	 */
	Elf32_Addr st_value;

	/**
	 * Many symbols have associated sizes.  This member holds zero
	   if the symbol has no size or an unknown size.
	 */
	uint32_t st_size;

	/**
	 * This member specifies the symbol's type and binding
       attributes:

              STT_NOTYPE
                     The symbol's type is not defined.

              STT_OBJECT
                     The symbol is associated with a data object.

              STT_FUNC
                     The symbol is associated with a function or other
                     executable code.

              STT_SECTION
                     The symbol is associated with a section.  Symbol
                     table entries of this type exist primarily for
                     relocation and normally have STB_LOCAL bindings.

              STT_FILE
                     By convention, the symbol's name gives the name of
                     the source file associated with the object file.  A
                     file symbol has STB_LOCAL bindings, its section
                     index is SHN_ABS, and it precedes the other
                     STB_LOCAL symbols of the file, if it is present.

              STT_LOPROC
              STT_HIPROC
                     Values in the inclusive range [STT_LOPROC,
                     STT_HIPROC] are reserved for processor-specific
                     semantics.

              STB_LOCAL
                     Local symbols are not visible outside the object
                     file containing their definition.  Local symbols of
                     the same name may exist in multiple files without
                     interfering with each other.

              STB_GLOBAL
                     Global symbols are visible to all object files being
                     combined.  One file's definition of a global symbol
                     will satisfy another file's undefined reference to
                     the same symbol.

              STB_WEAK
                     Weak symbols resemble global symbols, but their
                     definitions have lower precedence.

              STB_LOPROC
              STB_HIPROC
                     Values in the inclusive range [STB_LOPROC,
                     STB_HIPROC] are reserved for processor-specific
                     semantics.

              There are macros for packing and unpacking the binding and
              type fields:

              ELF32_ST_BIND(info)
              ELF64_ST_BIND(info)
                     Extract a binding from an st_info value.

              ELF32_ST_TYPE(info)
              ELF64_ST_TYPE(info)
                     Extract a type from an st_info value.

              ELF32_ST_INFO(bind, type)
              ELF64_ST_INFO(bind, type)
                     Convert a binding and a type into an st_info value.
	 */
	unsigned char st_info;

	/**
	 * This member defines the symbol visibility.

              STV_DEFAULT
                     Default symbol visibility rules.  Global and weak
                     symbols are available to other modules; references
                     in the local module can be interposed by definitions
                     in other modules.
              STV_INTERNAL
                     Processor-specific hidden class.
              STV_HIDDEN
                     Symbol is unavailable to other modules; references
                     in the local module always resolve to the local
                     symbol (i.e., the symbol can't be interposed by
                     definitions in other modules).
              STV_PROTECTED
                     Symbol is available to other modules, but references
                     in the local module always resolve to the local
                     symbol.

              There are macros for extracting the visibility type:

              ELF32_ST_VISIBILITY(other) or ELF64_ST_VISIBILITY(other)
	 */
	unsigned char st_other;

	/**
	 * Every symbol table entry is "defined" in relation to some
       section.  This member holds the relevant section header
       table index.
	 */
	uint16_t st_shndx;
};


void init(int argc, char** argv)
{
	_argc = argc;
	_argv = argv;
}

static FILE* f = nullptr;

static const char* lookup_symbol(const Elf32_Sym& symbol, const elf_section_header& string_section_header)
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

	elf_header file_header;

	static_assert(offsetof(elf_header, e_shoff) == 0x20);
	static_assert(offsetof(elf_section_header, sh_offset) == 0x10);

	fread((char*)&file_header, sizeof(file_header), 1, f);

	fseek(f, file_header.e_shoff, SEEK_SET);

	// printf("Num sections: %d\n", file_header.e_shnum);
	// printf("String section num: %d\n", file_header.e_shstrndx);

	if (file_header.e_shentsize != sizeof(elf_section_header))
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
		elf_section_header section_header;
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
