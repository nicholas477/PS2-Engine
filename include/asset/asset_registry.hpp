#pragma once

#include <egg/filesystem.hpp>
#include <egg/asset.hpp>

namespace AssetRegistry
{
struct Asset
{
	std::unique_ptr<std::byte[]> data;
	size_t size;
};

bool get_asset(::Asset::Reference asset, Asset*& out_asset, u16 alignment = 0, bool load_if_not_found = true);
bool get_asset(const Filesystem::Path& path, Asset*& out_asset, u16 alignment = 0, bool load_if_not_found = true);

//bool unload_asset(::Asset::Reference asset);
}; // namespace AssetRegistry