#pragma once

#include "tamtypes.h"
#include "packet2_utils.h"

#include <utility>

namespace egg::ps2::graphics
{

struct texture_descriptor
{
	texture_descriptor();

	lod_t lod;
	clutbuffer_t clut;
	texbuffer_t t_texbuff;
	texwrap_t wrap;

	bool is_uploaded;

	u32 height;

	void set_width_height(u32 width, u32 height);

	std::pair<u32, u32> get_width_height() const;
};

// Uploads the texture to vram
void upload_texture(texture_descriptor& texture, void* texture_data, void* clut_data);

void unload_texture(texture_descriptor& texture);

// Sets the texture currently being drawn
bool set_texture(texture_descriptor& texture);
} // namespace egg::ps2::graphics