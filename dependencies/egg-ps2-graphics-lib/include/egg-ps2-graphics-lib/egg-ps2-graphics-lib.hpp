#pragma once

#include <packet2.h>
#include <gs_psm.h>

#include <array>
#include "types.hpp"

struct Matrix;
struct Vector;

namespace egg::ps2::graphics
{
struct init_options
{
	init_options();

	// Defaults to 640
	u32 framebuffer_width;

	// Defaults to 448
	u32 framebuffer_height;

	// Framebuffer color depth, defaults to 32bpp
	u32 framebuffer_psm;

	// Screen mode, defaults to NTSC (256 x 224 to 640 x 448)
	//
	// If you change the framebuffer resolution, you should probably change this
	// too
	u32 graph_mode;

	// Defaults to non-interlaced (0)
	u32 interlaced;

	// Frame or field mode. Honestly not sure what this does, I think it
	// refers to interlacing.
	//
	// Defaults to frame (1)
	u32 ffmd;

	// Enable/disable flicker filter (for interlacing)
	u32 flicker_filter;

	bool double_buffer_vu;
};

void init(const init_options& init_options = init_options());

// Uploads a VU program, returns the address of the loaded program
u32 load_vu_program(void* program_start_address, void* program_end_address);

static u32 load_vu_program(std::pair<void*, void*> program_addresses)
{
	return load_vu_program(program_addresses.first, program_addresses.second);
}

void clear_screen(int r, int g, int b);

void wait_vsync();

void start_draw();
void end_draw();

using vif_packet_t = utils::inline_packet2<20000>;

packet2_t* get_current_vif_packet();

} // namespace egg::ps2::graphics