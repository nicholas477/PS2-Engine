#include "asset/asset_registry.hpp"

#include <unordered_map>

namespace AssetRegistry
{
std::unordered_map<::Asset::Reference, Asset> assets;

bool get_asset(::Asset::Reference asset, Asset*& out_asset, u16 alignment, bool load_if_not_found)
{
	auto itr = assets.find(asset);
	if (itr != assets.end())
	{
		out_asset = &itr->second;
		return true;
	}
	else
	{
		if (load_if_not_found)
		{
			Asset new_asset;

			const Filesystem::Path& path = ::Asset::lookup_path(asset);
			Filesystem::load_file(path, new_asset.data, new_asset.size, alignment);

			auto itr = assets.emplace(asset, std::move(new_asset));

			out_asset = &itr.first->second;
			return true;
		}
	}

	return false;
}

bool get_asset(const Filesystem::Path& path, Asset*& out_asset, u16 alignment, bool load_if_not_found)
{
	const auto asset = ::Asset::Reference(path);

	auto itr = assets.find(asset);
	if (itr != assets.end())
	{
		out_asset = &itr->second;
		return true;
	}
	else
	{
		if (load_if_not_found)
		{
			Asset new_asset;

			Filesystem::load_file(path, new_asset.data, new_asset.size, alignment);

			auto itr = assets.emplace(asset, std::move(new_asset));

			out_asset = &itr.first->second;
			return true;
		}
	}

	return false;
}

bool unload_asset(::Asset::Reference asset)
{
	return assets.erase(asset) > 0;
}

size_t unload_all_assets()
{
	const size_t num_assets = assets.size();
	assets.clear();
	return num_assets;
}
} // namespace AssetRegistry