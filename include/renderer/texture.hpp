#pragma once

#include "asset/asset_registry.hpp"
#include "utils/debuggable.hpp"
#include "egg-ps2-graphics-lib/texture.hpp"

#include <egg/asset.hpp>

struct Texture: public Debuggable
{
	Texture();

	Texture(Asset::Reference texture_asset_ref, AssetRegistry::Asset* in_texture_asset);

	// Allocates and uploads the texture to gs memory
	bool upload_texture();

	// Deallocates the texture from gs memory
	bool unload_texture();

	AssetRegistry::Asset* texture_asset;

	class TextureFileHeader* get_texture() const;
	int get_texture_size_pages() const;

	egg::ps2::graphics::texture_descriptor texture_descriptor;

	// for debug
	const Filesystem::Path* path;
	virtual const char* get_type_name() const { return typeid(Texture).name(); }
};

Texture* get_texture(Asset::Reference asset, bool load_if_not_found = true);