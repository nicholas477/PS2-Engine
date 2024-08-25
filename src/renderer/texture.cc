#include "renderer/texture.hpp"

namespace GS
{
Texture::Texture()
{
	texture_asset = nullptr;
	path          = nullptr;
	debug_name    = "uninitialized texture";
}

Texture::Texture(Asset::Reference texture_asset_ref)
{
	load_from_asset_ref(texture_asset_ref);
	debug_name = Asset::lookup_path(texture_asset_ref).data();
}

void Texture::load_from_asset_ref(Asset::Reference texture_asset_ref)
{
	check(AssetRegistry::get_asset(texture_asset_ref, texture_asset, 16, true));
}
} // namespace GS