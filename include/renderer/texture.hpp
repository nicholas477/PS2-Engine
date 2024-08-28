#pragma once

#include "asset/asset_registry.hpp"
#include "utils/debuggable.hpp"

#include <egg/asset.hpp>

namespace GS
{
struct Texture: public Debuggable
{
	Texture();

	Texture(Asset::Reference texture_asset_ref);

	void load_from_asset_ref(Asset::Reference texture_asset_ref);

	AssetRegistry::Asset* texture_asset;

	// for debug
	const Filesystem::Path* path;
	virtual const char* get_type_name() const { return typeid(Texture).name(); }
};
} // namespace GS