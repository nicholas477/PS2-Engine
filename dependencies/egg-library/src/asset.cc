#include "egg/asset.hpp"

static HashMap<Asset::asset_hashmap_size, Asset::Reference, Filesystem::Path> asset_paths;

const Filesystem::Path& Asset::lookup_path(Reference reference)
{
	return asset_paths[reference];
}

bool Asset::add_path(Asset::Reference reference, const Filesystem::Path& path)
{
	return asset_paths.add_key_value(reference, path);
}

bool Asset::add_path(const Filesystem::Path& path)
{
	return add_path(Asset::Reference(path), path);
}


bool Asset::add_path(Reference reference, const char* path, bool convert_path)
{
	check(strlen(path) <= 255);
	return add_path(reference, Filesystem::Path(path, convert_path));
}

void Asset::load_asset_table(std::byte* bytes, size_t length)
{
	check(sizeof(AssetHashMapT) == length);

	memcpy(&asset_paths, bytes, sizeof(AssetHashMapT));
}

void Asset::serialize_asset_table(std::vector<std::byte>& out_bytes)
{
	out_bytes.resize(sizeof(AssetHashMapT));

	memcpy(out_bytes.data(), &asset_paths, sizeof(AssetHashMapT));
}

Asset::AssetHashMapT& Asset::get_asset_table()
{
	return asset_paths;
}