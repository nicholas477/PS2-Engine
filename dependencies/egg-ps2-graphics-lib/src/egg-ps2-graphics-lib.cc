#include "egg-ps2-graphics-lib/egg-ps2-graphics-lib.hpp"
#include "egg-ps2-graphics-lib/types.hpp"
#include "egg-ps2-graphics-lib/vu_programs.hpp"
#include "egg-ps2-graphics-lib/gs_mem.hpp"

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
void init_gs(const init_options& init_options, framebuffer_t* t_frame, zbuffer_t* t_z)
{
	printf("egg-ps2-graphics-lib: init gs\n");

	// Allocate the two framebuffers
	t_frame->width  = init_options.framebuffer_width;
	t_frame->height = init_options.framebuffer_height;
	t_frame->mask   = 0;
	t_frame->psm    = init_options.framebuffer_psm;

	// Allocate some vram for our framebuffer.
	t_frame->address = gs_mem::allocate_framebuffer(t_frame->width, t_frame->height, t_frame->psm, GRAPH_ALIGN_PAGE);

	t_frame++;

	t_frame->width  = init_options.framebuffer_width;
	t_frame->height = init_options.framebuffer_height;
	t_frame->mask   = 0;
	t_frame->psm    = init_options.framebuffer_psm;

	// Allocate some vram for our framebuffer.
	t_frame->address = gs_mem::allocate_framebuffer(t_frame->width, t_frame->height, t_frame->psm, GRAPH_ALIGN_PAGE);

	// Enable the zbuffer.
	t_z->enable  = DRAW_ENABLE;
	t_z->mask    = 0;
	t_z->method  = ZTEST_METHOD_GREATER_EQUAL;
	t_z->zsm     = GS_ZBUF_32;
	t_z->address = gs_mem::allocate_framebuffer(t_frame->width, t_frame->height, t_z->zsm, GRAPH_ALIGN_PAGE);

	gs_mem::finish_allocating_framebuffers();

	// Initialize the screen and tie the first framebuffer to the read circuits.
	graph_set_mode(init_options.interlaced, init_options.graph_mode, init_options.ffmd, init_options.flicker_filter);

	graph_set_screen(0, 0, t_frame->width, t_frame->height);
	graph_set_framebuffer_filtered(t_frame->address, t_frame->width, t_frame->psm, 0, 0);
	graph_enable_output();
}

/** Some initialization of GS 2 */
void init_drawing_environment(framebuffer_t* t_frame, zbuffer_t* t_z)
{
	printf("egg-ps2-graphics-lib: init_drawing_environment\n");
	utils::inline_packet2<20> packet2(P2_TYPE_NORMAL, P2_MODE_NORMAL, 1);

	// This will setup a default drawing environment.
	packet2_update(packet2, draw_setup_environment(packet2->next, 0, t_frame, t_z));

	// Now reset the primitive origin to 2048-width/2,2048-height/2.
	packet2_update(packet2, draw_primitive_xyoffset(packet2->next, 0, (2048 - (frame->width / 2.f)), (2048 - (frame->height / 2.f))));

	// Finish setting up the environment.
	packet2_update(packet2, draw_finish(packet2->next));

	// Now send the packet, no need to wait since it's the first.
	dma_channel_send_packet2(packet2, DMA_CHANNEL_GIF, 1);
	dma_channel_wait(DMA_CHANNEL_GIF, 0);
}

void vu1_set_double_buffer_settings()
{
	printf("egg-ps2-graphics-lib: vu1_set_double_buffer_settings\n");
	utils::inline_packet2<2> double_buffer_pkt(P2_TYPE_NORMAL, P2_MODE_CHAIN, 1);
	packet2_utils_vu_add_double_buffer(double_buffer_pkt, 8, 496);
	packet2_utils_vu_add_end_tag(double_buffer_pkt);

	dma_channel_send_packet2(double_buffer_pkt, DMA_CHANNEL_VIF1, 1);
	dma_channel_wait(DMA_CHANNEL_VIF1, 0);
}

void flip_buffers(framebuffer_t* t_frame)
{
	static utils::inline_packet2<8> flip(P2_TYPE_UNCACHED_ACCL, P2_MODE_NORMAL, 0);
	packet2_reset(flip, 0);
	packet2_update(flip, draw_framebuffer(flip->next, 0, t_frame));
	packet2_update(flip, draw_finish(flip->next));

	dma_channel_wait(DMA_CHANNEL_GIF, 0);
	dma_channel_send_packet2(flip, DMA_CHANNEL_GIF, 0);

	draw_wait_finish();
}

static vif_packet_t vif_packet;

} // namespace

namespace egg::ps2::graphics
{

init_options::init_options()
{
	framebuffer_width  = 640;
	framebuffer_height = 448;

	framebuffer_psm = GS_PSM_32;

	graph_mode     = GRAPH_MODE_NTSC;
	interlaced     = GRAPH_MODE_NONINTERLACED;
	ffmd           = GRAPH_MODE_FRAME;
	flicker_filter = GRAPH_DISABLE;

	double_buffer_vu = true;
}

static u32 current_program_addr = 0;

void init(const init_options& init_options)
{
	// Init DMA channels.
	dma_channel_initialize(DMA_CHANNEL_GIF, NULL, 0);
	dma_channel_initialize(DMA_CHANNEL_VIF1, NULL, 0);
	dma_channel_fast_waits(DMA_CHANNEL_GIF);
	dma_channel_fast_waits(DMA_CHANNEL_VIF1);

	current_program_addr = 0;

	if (init_options.double_buffer_vu)
	{
		vu1_set_double_buffer_settings();
	}

	// Init the GS, framebuffer, zbuffer
	init_gs(init_options, frame, &z);

	init_drawing_environment(frame, &z);

	// Load the kick program
	vu1_programs::get_kick_program_addr() = load_vu_program(vu1_programs::get_kick_program_mem_address());

	current_frame = frame;

	vif_packet.initialize(P2_TYPE_NORMAL, P2_MODE_CHAIN, 1);
}

u32 load_vu_program(void* program_start_address, void* program_end_address)
{
	printf("Program size (bytes): %d\n", (ptrdiff_t)program_end_address - (ptrdiff_t)program_start_address);

	const u32 packet_size  = packet2_utils_get_packet_size_for_program((u32*)program_start_address, (u32*)program_end_address) + 1;
	const u32 program_size = (packet_size * 512) / 16;

	// Make sure we aren't going to run out of memory before we load it into memory
	assert((current_program_addr + program_size) <= 1024);

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
	printf("Free size (qwords): %u\n\n", 1024 - current_program_addr);

	return current_program_addr - program_size;
}

void clear_screen(int r, int g, int b)
{
	utils::inline_packet2<35> clear(P2_TYPE_NORMAL, P2_MODE_NORMAL, 0);

	// Clear framebuffer but don't update zbuffer.
	packet2_update(clear, draw_disable_tests(clear->next, 0, &z));
	packet2_update(clear, draw_clear(clear->next, 0, 2048.0f - (current_frame->width / 2.f), 2048.0f - (current_frame->height / 2.f), current_frame->width, current_frame->height, r, g, b));
	packet2_update(clear, draw_enable_tests(clear->next, 0, &z));
	packet2_update(clear, draw_finish(clear->next));

	// Now send our current dma chain.
	dma_channel_send_packet2(clear, DMA_CHANNEL_GIF, 1);
}

void wait_vsync()
{
	graph_wait_vsync();

	graph_set_framebuffer_filtered(current_frame->address, current_frame->width, current_frame->psm, 0, 0);

	context ^= 1;
	current_frame = &frame[context];

	flip_buffers(current_frame);
}

void start_draw()
{
	//printf("start_draw.........\n");

	packet2_reset(get_current_vif_packet(), 0);
}

void end_draw()
{
	//printf("end_draw.........\n");

	packet2_utils_vu_add_end_tag(get_current_vif_packet());
	dma_channel_wait(DMA_CHANNEL_VIF1, 0);
	dma_channel_send_packet2(get_current_vif_packet(), DMA_CHANNEL_VIF1, 1);
	dma_channel_wait(DMA_CHANNEL_VIF1, 0);

	draw_wait_finish();

	return;

	// dma_channel_wait(DMA_CHANNEL_VIF1, 0);
	// dma_channel_send_packet2(draw_finish_packet, DMA_CHANNEL_VIF1, true);

	// while (!(*GS_REG_CSR & 2))
	// {
	// }

	// *GS_REG_CSR |= 2;
}

packet2_t* get_current_vif_packet()
{
	return vif_packet;
}

} // namespace egg::ps2::graphics
