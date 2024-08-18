#include "egg-ps2-graphics-lib/egg-ps2-graphics-lib.hpp"
#include "egg-ps2-graphics-lib/types.hpp"
#include "egg-ps2-graphics-lib/vu_programs.hpp"

#include <kernel.h>
#include <malloc.h>
#include <tamtypes.h>
#include <gs_psm.h>
#include <dma.h>
#include <packet2.h>
#include <packet2_utils.h>
#include <graph.h>
#include <draw.h>
#include <gs_privileged.h>

#include "egg/math_types.hpp"

using namespace egg::ps2::graphics;

namespace
{
// Which framebuffer we are drawing to
int context = 0;

// The buffers to be used.
framebuffer_t frame[2];
framebuffer_t* current_frame;
zbuffer_t z;

/** Some initialization of GS and VRAM allocation */
void init_gs(framebuffer_t* t_frame, zbuffer_t* t_z)
{
	printf("egg-ps2-graphics-lib: init gs\n");
	// Define a 32-bit 640x512 framebuffer.
	t_frame->width  = 640;
	t_frame->height = 512;
	t_frame->mask   = 0;
	t_frame->psm    = GS_PSM_32;

	// Allocate some vram for our framebuffer.
	t_frame->address = graph_vram_allocate(t_frame->width, t_frame->height, t_frame->psm, GRAPH_ALIGN_PAGE);

	t_frame++;

	t_frame->width  = 640;
	t_frame->height = 512;
	t_frame->mask   = 0;
	t_frame->psm    = GS_PSM_32;

	// Allocate some vram for our framebuffer.
	t_frame->address = graph_vram_allocate(t_frame->width, t_frame->height, t_frame->psm, GRAPH_ALIGN_PAGE);

	// Enable the zbuffer.
	t_z->enable  = DRAW_ENABLE;
	t_z->mask    = 0;
	t_z->method  = ZTEST_METHOD_GREATER_EQUAL;
	t_z->zsm     = GS_ZBUF_32;
	t_z->address = graph_vram_allocate(t_frame->width, t_frame->height, t_z->zsm, GRAPH_ALIGN_PAGE);

	// Initialize the screen and tie the first framebuffer to the read circuits.
	graph_initialize(t_frame->address, t_frame->width, t_frame->height, t_frame->psm, 0, 0);
}

/** Some initialization of GS 2 */
void init_drawing_environment(framebuffer_t* t_frame, zbuffer_t* t_z)
{
	printf("egg-ps2-graphics-lib: init_drawing_environment\n");
	utils::inline_packet2<20> packet2(P2_TYPE_NORMAL, P2_MODE_NORMAL, 1);

	// This will setup a default drawing environment.
	packet2_update(packet2, draw_setup_environment(packet2->next, 0, t_frame, t_z));

	// Now reset the primitive origin to 2048-width/2,2048-height/2.
	packet2_update(packet2, draw_primitive_xyoffset(packet2->next, 0, (2048 - 320), (2048 - 256)));

	// Finish setting up the environment.
	packet2_update(packet2, draw_finish(packet2->next));

	// Now send the packet, no need to wait since it's the first.
	dma_channel_send_packet2(packet2, DMA_CHANNEL_GIF, 1);
	dma_wait_fast();
}

void vu1_set_double_buffer_settings()
{
	printf("egg-ps2-graphics-lib: vu1_set_double_buffer_settings\n");
	utils::inline_packet2<1> packet2(P2_TYPE_NORMAL, P2_MODE_CHAIN, 1);
	//packet2_utils_vu_add_double_buffer(packet2, 0, 500);
	packet2_utils_vu_add_end_tag(packet2);
	dma_channel_send_packet2(packet2, DMA_CHANNEL_VIF1, 1);
	dma_channel_wait(DMA_CHANNEL_VIF1, 0);
}

void flip_buffers(framebuffer_t* t_frame)
{
	utils::inline_packet2<3> flip(P2_TYPE_NORMAL, P2_MODE_NORMAL, 0);
	packet2_update(flip, draw_framebuffer(flip->next, 0, t_frame));
	packet2_update(flip, draw_finish(flip->next));

	dma_wait_fast();
	dma_channel_send_packet2(flip, DMA_CHANNEL_GIF, 1);

	draw_wait_finish();
}

} // namespace

namespace egg::ps2::graphics
{

static u32 current_program_addr = 0;

void init()
{
	// Init DMA channels.
	dma_channel_initialize(DMA_CHANNEL_GIF, NULL, 0);
	dma_channel_initialize(DMA_CHANNEL_VIF1, NULL, 0);
	dma_channel_fast_waits(DMA_CHANNEL_GIF);
	dma_channel_fast_waits(DMA_CHANNEL_VIF1);

	current_program_addr = 0;

	vu1_set_double_buffer_settings();

	// Init the GS, framebuffer, zbuffer
	init_gs(frame, &z);

	init_drawing_environment(frame, &z);

	const auto [draw_finish_start, draw_finish_end] = vu1_programs::get_draw_finish_program_mem_address();

	vu1_programs::get_draw_finish_program_addr() = load_vu_program(draw_finish_start, draw_finish_end);

	current_frame = frame;
}

u32 load_vu_program(void* program_start_address, void* program_end_address)
{
	const u32 packet_size  = packet2_utils_get_packet_size_for_program((u32*)program_start_address, (u32*)program_end_address) + 1;
	const u32 program_size = (packet_size * 512) / 16;

	// Make sure we aren't going to run out of memory before we load it into memory
	assert((current_program_addr + program_size) <= 1000);

	packet2_t* packet2 = packet2_create(packet_size, P2_TYPE_NORMAL, P2_MODE_CHAIN, 1);
	packet2_vif_add_micro_program(packet2, current_program_addr, (u32*)program_start_address, (u32*)program_end_address);
	packet2_utils_vu_add_end_tag(packet2);

	dma_channel_wait(DMA_CHANNEL_VIF1, 0);
	dma_channel_send_packet2(packet2, DMA_CHANNEL_VIF1, 1);

	dma_channel_wait(DMA_CHANNEL_VIF1, 0);
	packet2_free(packet2);

	current_program_addr += program_size;

	printf("Loaded vu program at: %u\n", current_program_addr - program_size);
	printf("Program size (qwords): %u\n", program_size);

	return current_program_addr - program_size;
}

void clear_screen(int r, int g, int b)
{
	utils::inline_packet2<35> clear(P2_TYPE_NORMAL, P2_MODE_NORMAL, 0);

	// Clear framebuffer but don't update zbuffer.
	packet2_update(clear, draw_disable_tests(clear->next, 0, &z));
	packet2_update(clear, draw_clear(clear->next, 0, 2048.0f - 320.f, 2048.0f - 256.f, current_frame->width, current_frame->height, r, g, b));
	packet2_update(clear, draw_enable_tests(clear->next, 0, &z));
	packet2_update(clear, draw_finish(clear->next));

	// Now send our current dma chain.
	dma_wait_fast();
	dma_channel_send_packet2(clear, DMA_CHANNEL_GIF, 1);

	// Wait for scene to finish drawing
	draw_wait_finish();
}

void wait_vsync()
{
	//printf("waiting vsync.........\n");
	graph_wait_vsync();

	graph_set_framebuffer_filtered(current_frame->address, current_frame->width, current_frame->psm, 0, 0);

	context ^= 1;
	current_frame = &frame[context];

	flip_buffers(current_frame);
}

void start_draw()
{
	//printf("start_draw.........\n");
}

void end_draw()
{
	return;

	static utils::inline_packet2<10> finish(P2_TYPE_NORMAL, P2_MODE_NORMAL, 1);
	prim_t prim;
	prim.type         = PRIM_TRIANGLE;
	prim.shading      = PRIM_SHADE_GOURAUD;
	prim.mapping      = 1;
	prim.fogging      = 0;
	prim.blending     = 1;
	prim.antialiasing = 0;
	prim.mapping_type = PRIM_MAP_ST;
	prim.colorfix     = PRIM_UNFIXED;

	packet2_utils_vu_open_unpack(finish, 10, true);
	{
		packet2_utils_gif_add_set(finish, 1);
		packet2_utils_gs_add_draw_finish_giftag(finish);
		packet2_utils_gs_add_prim_giftag(finish, &prim, 0,
		                                 ((u64)GIF_REG_RGBAQ) << 0, 1, 0);
	}
	packet2_utils_vu_close_unpack(finish);

	packet2_utils_vu_add_start_program(finish, vu1_programs::get_draw_finish_program_addr());
	packet2_utils_vu_add_end_tag(finish);

	dma_channel_wait(DMA_CHANNEL_VIF1, 0);
	dma_channel_send_packet2(finish, DMA_CHANNEL_VIF1, true);

	while (!(*GS_REG_CSR & 2))
	{
	}

	*GS_REG_CSR |= 2;
}

} // namespace egg::ps2::graphics
