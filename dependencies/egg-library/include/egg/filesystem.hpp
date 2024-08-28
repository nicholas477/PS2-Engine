#pragma once

#include <cstddef>
#include <vector>
#include <string.h>
#include <string>
#include <string_view>
#include <memory>
#include <functional>
#include <array>
#include <cstdint>
#include <unordered_set>

#include "egg/assert.hpp"
#include "egg/hashing.hpp"
#include "egg/string.hpp"
#include "egg/hashmap.hpp"
#include "egg/offset_pointer.hpp"
#include "egg/serialization.hpp"

#ifdef _MSC_VER
#include <stdlib.h> // for WCSTOMBS
#endif


namespace Filesystem
{
constexpr size_t max_path_length = 128;

struct Path;
enum class Type {
	uninitialized = 0, // default value, if any filesystem functions are run
	                   // before initialization then it will crash
	cdrom,
	host,
	rom
};

Type get_filesystem_type();
void set_filesystem_type(Type new_type);

static constexpr const char* get_filesystem_prefix(Type in_filesystem_type = get_filesystem_type())
{
	switch (in_filesystem_type)
	{
		case Type::cdrom:
			return "cdrom0:\\";
		case Type::host:
			return "host0:";
		case Type::rom:
			return "rom0:";

		case Type::uninitialized:
			check(false);
			break;
	}

	return 0;
}

static constexpr char get_filesystem_separator(Type in_filesystem_type = get_filesystem_type())
{
	switch (in_filesystem_type)
	{
		case Type::cdrom:
			return '\\';
		case Type::host:
		case Type::rom:
			return '/';

		case Type::uninitialized:
			check(false);
			break;
	}

	return '\0';
}

// Returns true if the file was loaded succesfully
bool load_file(const Path& path, std::vector<std::byte>& out_bytes);
bool load_file(const Path& path, std::unique_ptr<std::byte[]>& out_bytes, size_t& size, size_t alignment = 1);
bool file_exists(const Path& path);
//void iterate_dir(const Path& dir, std::function<void(const Path&)> itr_func, bool recursive = false);

constexpr struct Path convert_to_iso_path(const char* path);

struct Path
{
public:
	constexpr Path() noexcept
	    : mem({0})
	{
	}

	constexpr Path(const char* in_path, bool convert_path = true)
	    : mem({0})
	{
		if (convert_path)
		{
			*this = convert_to_iso_path(in_path);
		}
		else
		{
			*this = in_path;
		}
	}

#ifdef _MSC_VER
	Path(const wchar_t* in_path, bool convert_path = true)
	{
		wcstombs(mem.data(), in_path, max_path_length);
		if (convert_path)
		{
			*this = convert_to_iso_path(mem.data());
		}
	}
#endif

	constexpr char& operator[](size_t id) noexcept
	{
		return mem[id];
	}
	constexpr const char& operator[](size_t id) const noexcept { return mem[id]; }

	// Returns the raw data contained in mem. This is not a full system filepath
	constexpr const char* data() const noexcept { return mem.data(); }
	constexpr size_t max_size() const noexcept { return max_path_length; }
	constexpr size_t length() const { return constexpr_strlen(mem.data()); }

	constexpr bool operator==(const char* rhs)
	{
		return std::string_view(mem.data()) == std::string_view(rhs);
	}

	const char* to_full_filepath(Type in_filesystem_type = get_filesystem_type()) const;

	constexpr uint32_t hash() const
	{
		return crc32(data(), length());
	}

	std::array<char, 256> mem;

protected:
	constexpr Path& operator=(const char* rhs)
	{
		const size_t copy_amt = std::min(max_path_length - 1, constexpr_strlen(rhs));
		for (size_t i = 0; i < copy_amt; ++i)
		{
			mem[i] = rhs[i];
		}

		return *this;
	}
};

// Converts a filepath to semi-ISO 9660 8.3 (PS2 ISO format)
//
// 1. The entire path to converted to uppercase, the directory separator ('\') is
// 	  converted to backslashes.
// 2. The directories are limited to 8 characters
// 3. The filename is limited to 8 characters with a 3 character extension limit.
// 4. Also any prefixed slashes are removed
//
// For example: /assets/sounds/vine-boom.adpcm -> ASSETS\SOUNDS\VINE_BOO.ADP
constexpr Path convert_to_iso_path(const char* path)
{
	Path out_path;

	// iterate past the beginning separators
	while (path[0] == '\\' || path[0] == '/' || path[0] == '.')
	{
		path++;
	}

	// Iterate backwards and look for the beginning of the filename
	size_t filename_begin  = 0;
	size_t extension_begin = constexpr_strlen(path);
	for (size_t i = constexpr_strlen(path);; --i)
	{
		if (path[i] == '.')
		{
			extension_begin = i;
		}
		else if (path[i] == '/' || path[i] == '\\')
		{
			filename_begin = i + 1;
			break;
		}

		if (i == 0)
		{
			break;
		}
	}

	// Copy up to the first 8 filename characters over
	size_t itr_end = std::min(std::min(filename_begin + 8, extension_begin), constexpr_strlen(path));
	for (size_t i = 0; i < itr_end; ++i)
	{
		if (path[i] == '\\' || path[i] == '/')
		{
			out_path[i] = get_filesystem_separator(Type::cdrom);
		}
		else if (path[i] == '-')
		{
			out_path[i] = '_';
		}
		else
		{
			out_path[i] = charToUpper(path[i]);
		}
	}

	// Copy the extension over
	size_t itr_extension_end = std::min((size_t)4U, constexpr_strlen(path) - extension_begin);
	for (size_t i = 0; i < itr_extension_end; ++i)
	{
		if (path[extension_begin + i] == '-')
		{
			out_path[itr_end + i] = '_';
		}
		else
		{
			out_path[itr_end + i] = charToUpper(path[extension_begin + i]);
		}
	}
	out_path[itr_end + itr_extension_end] = '\0';

	return out_path;
}

#ifndef _MSC_VER
static_assert(convert_to_iso_path("/asdf-ggggg/sdkfjs.egg2") == "ASDF_GGGGG\\SDKFJS.EGG");
static_assert(convert_to_iso_path("MANIFEST.ISO") == "MANIFEST.ISO");
#endif

} // namespace Filesystem

// Path literal constructor
//
// Converts the path to a system path. I.e: assets/sounds/vine-boom.adpcm -> ASSETS\SOUNDS\VINE_BOO.ADP
// Look at: Filesystem::convert_to_83_path()
constexpr Filesystem::Path operator""_p(const char* p, std::size_t)
{
	return Filesystem::Path(p, true);
}

template <>
struct std::hash<Filesystem::Path>
{
	std::size_t operator()(const Filesystem::Path& k) const
	{
		return k.hash();
	}
};

namespace Asset
{
// Reference to an on-disk asset. This is just a hash of it's ISO 9660 8.3
// filepath (see Filesystem::convert_to_iso_path)
struct Reference
{
	uint32_t hash;

	constexpr Reference()
	    : hash(0)
	{
	}

	constexpr Reference(uint32_t in_hash)
	    : hash(in_hash)
	{
	}

	constexpr Reference(const Filesystem::Path& path)
	    : hash(0)
	{
		hash = path.hash();
	}

	constexpr Reference(const char* path, bool convert_to_iso_path = true)
	    : hash(0)
	{
		hash = Filesystem::Path(path, convert_to_iso_path).hash();
	}

	constexpr bool operator==(const Reference& other) const
	{
		return hash == other.hash;
	}
};

constexpr size_t asset_hashmap_size = 128;
using AssetHashMapT                 = HashMap<asset_hashmap_size, Reference, Filesystem::Path>;

// Note: this will crash if the path does not exist
const Filesystem::Path& lookup_path(Reference reference);

// Returns true if it succesfully added the path
bool add_path(Reference reference, const Filesystem::Path& path);

// Returns true if it succesfully added the path
// This version hashes the path before adding it
bool add_path(const Filesystem::Path& path);

bool add_path(Reference reference, const char* path, bool convert_path);

// Copies the passed in asset table bytes to the global asset table
// Checks to make sure length matches the length of the global asset table
void load_asset_table(std::byte* bytes, size_t length);

// Writes out the global asset table
void serialize_asset_table(std::vector<std::byte>& out_bytes);

AssetHashMapT& get_asset_table();

void set_asset_table_loaded(bool loaded);
} // namespace Asset

template <>
struct std::hash<Asset::Reference>
{
	std::size_t operator()(const Asset::Reference& k) const
	{
		return k.hash;
	}
};

// Overload this to add reference collection to your own types
static void collect_references(std::unordered_set<Asset::Reference>& references, const ::OffsetArray<Asset::Reference>& arr)
{
	for (Asset::Reference ref : arr)
	{
		references.insert(ref);
	}
}

// Asset literal constructor
//
// Converts the path to a asset reference hash
constexpr Asset::Reference operator""_asset(const char* p, std::size_t)
{
	return Asset::Reference(p, true);
}

namespace Filesystem
{
static bool load_file(Asset::Reference reference, std::unique_ptr<std::byte[]>& out_bytes, size_t& size, size_t alignment = 1)
{
	return load_file(Asset::lookup_path(reference), out_bytes, size, alignment);
}
} // namespace Filesystem