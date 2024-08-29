#include "renderer/texture.hpp"

#include <unordered_map>
#include "asset/asset_registry.hpp"
#include "egg/texture_header.hpp"
#include "egg-ps2-graphics-lib/gs_mem.hpp"
#include "gs_psm.h"

static std::unordered_map<Asset::Reference, Texture> textures;

Texture::Texture()
{
	texture_asset      = nullptr;
	path               = nullptr;
	texture_descriptor = egg::ps2::graphics::texture_descriptor();
	debug_name         = "uninitialized texture";
}

Texture::Texture(Asset::Reference texture_asset_ref, AssetRegistry::Asset* in_texture_asset)
{
	texture_asset = in_texture_asset;
	debug_name    = Asset::lookup_path(texture_asset_ref).data();

	const TextureFileHeader* texture = get_texture();
	if (texture)
	{
		printf("Texture width height: %u, %u\n", texture->size_x, texture->size_y);

		texture_descriptor.t_texbuff.info.components = texture->components;
		texture_descriptor.t_texbuff.info.function   = texture->function;
		texture_descriptor.t_texbuff.psm             = texture->psm;

		texture_descriptor.clut.start        = 0;
		texture_descriptor.clut.psm          = GS_PSM_32;
		texture_descriptor.clut.load_method  = texture->clut.length > 0 ? CLUT_LOAD : CLUT_NO_LOAD;
		texture_descriptor.clut.storage_mode = CLUT_STORAGE_MODE1;

		texture_descriptor.wrap.horizontal = texture->horizontal;
		texture_descriptor.wrap.vertical   = texture->vertical;

		texture_descriptor.wrap.minu = texture->minu;
		texture_descriptor.wrap.maxu = texture->maxu;

		texture_descriptor.wrap.minv = texture->minv;
		texture_descriptor.wrap.maxv = texture->maxv;

		printf("Texture size (bytes): %ld\n", texture->data.length);

		texture_descriptor.set_width_height(texture->size_x, texture->size_y);
	}
}

bool Texture::upload_texture()
{
	TextureFileHeader* texture = get_texture();
	if (texture == nullptr)
	{
		return false;
	}

	// If we don't have an address to upload to, then find some memory slots to use
	if (texture_descriptor.t_texbuff.address == 0)
	{
		int gs_texture_address = egg::ps2::graphics::gs_mem::get_empty_texture_slot(get_texture_size_pages());
		check(gs_texture_address != -1);

		texture_descriptor.t_texbuff.address = gs_texture_address;
	}

	if (texture->clut.length > 0 && texture_descriptor.clut.address == 0)
	{
		int gs_clut_address = egg::ps2::graphics::gs_mem::get_empty_texture_slot(1);
		check(gs_clut_address != -1);

		texture_descriptor.clut.address = gs_clut_address;
	}

	// I don't know why but trying to call get_ptr on texture->data crashes the ps2
	// Fun!
	uint8_t* texture_data_ptr = (uint8_t*)&texture->data;
	void* texture_data        = texture_data_ptr + texture->data.offset;

	void* clut_data = texture->clut.length > 0 ? texture->clut.get_ptr() : nullptr;

	egg::ps2::graphics::upload_texture(texture_descriptor, texture_data, clut_data);

	return true;
}

bool Texture::unload_texture()
{
	if (texture_descriptor.t_texbuff.address != 0)
	{
		egg::ps2::graphics::gs_mem::unlock_texture_slot(texture_descriptor.t_texbuff.address);

		texture_descriptor.t_texbuff.address = 0;
	}

	if (texture_descriptor.clut.address != 0)
	{
		egg::ps2::graphics::gs_mem::unlock_texture_slot(texture_descriptor.clut.address);

		texture_descriptor.clut.address = 0;
	}

	return true;
}

TextureFileHeader* Texture::get_texture() const
{
	return reinterpret_cast<TextureFileHeader*>(texture_asset->data.get());
}

int Texture::get_texture_size_pages() const
{
	const TextureFileHeader* texture = get_texture();
	if (texture)
	{
		return (texture->data.length / 8192) + (texture->data.length % 8192) > 0 ? 1 : 0;
	}

	return -1;
}

Texture* get_texture(Asset::Reference asset, bool load_if_not_found)
{
	auto texture_itr = textures.find(asset);
	if (texture_itr != textures.end())
	{
		return &texture_itr->second;
	}
	else
	{
		AssetRegistry::Asset* texture_asset;

		AssetRegistry::get_asset(asset, texture_asset, 16, load_if_not_found);
		if (texture_asset)
		{
			textures[asset] = Texture(asset, texture_asset);

			return &textures[asset];
		}
	}

	return nullptr;
}