#pragma once

#include <cstdint>
#include <vector>
#include <unordered_set>

#include "egg/filesystem.hpp"
#include "egg/hashmap.hpp"
#include "egg/offset_pointer.hpp"

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