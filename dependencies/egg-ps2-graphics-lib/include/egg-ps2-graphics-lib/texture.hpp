#pragma once

#include "tamtypes.h"
#include "packet2_utils.h"

namespace egg::ps2::graphics
{

struct texture_descriptor
{
	texture_descriptor();

	lod_t lod;
	clutbuffer_t clut;
	texbuffer_t t_texbuff;

	bool is_uploaded;

	void set_width_height(u32 width, u32 height);
};

// Uploads the texture to vram
void upload_texture(texture_descriptor& texture, void* texture_data);

// Sets the texture currently being drawn
void set_texture(texture_descriptor& texture);
} // namespace egg::ps2::graphics