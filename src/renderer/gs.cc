#include <malloc.h>
#include <stdio.h>
#include <vector>
#include <forward_list>

#include "objects/camera.hpp"
#include "renderer/gs.hpp"
#include "input.hpp"
#include "renderer/renderable.hpp"
#include "stats.hpp"
#include "assert.hpp"

//#include <libgs.h>
#include <packet.h>

#include <dma_tags.h>
#include <gif_tags.h>
#include <gs_psm.h>

#include <dma.h>

#include <graph.h>

#include <draw.h>
#include <draw3d.h>

#include "renderer/path1.hpp"

namespace gs
{
// The buffers to be used.
static framebuffer_t frame[2];
static zbuffer_t z;

static const int screen_width  = 640;
static const int screen_height = 480;


static Path1& get_path1()
{
	static Path1 path1;
	return path1;
}

static void init_gs(framebuffer_t* frame, zbuffer_t* z)
{
	// Define a 32-bit framebuffer.
	frame->width  = screen_width;
	frame->height = screen_height;
	frame->mask   = 0;
	frame->psm    = GS_PSM_32;

	// Allocate some vram for our framebuffer.
	frame->address = graph_vram_allocate(frame->width, frame->height, frame->psm,
	                                     GRAPH_ALIGN_PAGE);

	frame++;

	frame->width  = screen_width;
	frame->height = screen_height;
	frame->mask   = 0;
	frame->psm    = GS_PSM_32;

	// Allocate some vram for our framebuffer.
	frame->address = graph_vram_allocate(frame->width, frame->height, frame->psm,
	                                     GRAPH_ALIGN_PAGE);

	// Enable the zbuffer.
	z->enable  = DRAW_ENABLE;
	z->mask    = 0;
	z->method  = ZTEST_METHOD_GREATER_EQUAL;
	z->zsm     = GS_ZBUF_32;
	z->address = graph_vram_allocate(frame->width, frame->height, z->zsm,
	                                 GRAPH_ALIGN_PAGE);

	// Initialize the screen and tie the first framebuffer to the read circuits.
	//graph_initialize(frame->address, frame->width, frame->height, frame->psm, 0,
	//                 0);

	graph_set_mode(GRAPH_MODE_NONINTERLACED, GRAPH_MODE_VGA_640_60, GRAPH_MODE_FRAME, GRAPH_DISABLE);
	graph_set_screen(0, 0, screen_width, screen_height);
	graph_set_bgcolor(0, 0, 0);
	graph_set_framebuffer_filtered(frame->address, frame->width, frame->psm, 0, 0);
	graph_enable_output();

	get_path1();
}

static void init_drawing_environment(framebuffer_t* frame, zbuffer_t* z)
{
	//packet2_inline<20> packet(P2_TYPE_NORMAL, P2_MODE_NORMAL, false);
	packet2 packet = packet2_create(20, P2_TYPE_NORMAL, P2_MODE_NORMAL, false);

	// This will setup a default drawing environment.
	packet.update(draw_setup_environment, 0, frame, z);

	// Now reset the primitive origin to 2048-width/2,2048-height/2.
	packet.update(draw_primitive_xyoffset, 0, (2048 - (screen_width / 2)), (2048 - (screen_height / 2)));

	// Finish setting up the environment.
	packet.update(draw_finish);

	// Now send the packet, no need to wait since it's the first.
	packet.send(DMA_CHANNEL_GIF, 1);
	dma_wait_fast();
}

static void flip_buffers(packet_t* flip, framebuffer_t* frame)
{
	qword_t* q = flip->data;

	q = draw_framebuffer(q, 0, frame);
	q = draw_finish(q);

	dma_wait_fast();
	dma_channel_send_normal_ucab(DMA_CHANNEL_GIF, flip->data, q - flip->data, 0);

	draw_wait_finish();
}

static int context = 0;

// Packets for doublebuffering dma sends
static std::array<packet2, 2> packets;

// This packet is special for framebuffer switching
static packet_t* flip_pkt;

static gs_state _gs_state;

std::array<packet2, 2>& gs_state::get_packets()
{
	return packets;
}

packet2& gs_state::get_current_packet()
{
	return packets[context];
}

void gs_state::flip_context()
{
	context ^= 1;
}

std::vector<renderable*>& get_renderables()
{
	static std::vector<renderable*> renderables;
	return renderables;
}

static void init_renderer(framebuffer_t* frame, zbuffer_t* z)
{
	packets[0] = packet2(1600, P2_TYPE_NORMAL, P2_MODE_CHAIN, true);
	packets[1] = packet2(1600, P2_TYPE_NORMAL, P2_MODE_CHAIN, true);

	// Uncached accelerated
	flip_pkt = packet_init(3, PACKET_UCAB);

	// Create the view_screen matrix.
	create_view_screen(_gs_state.view_screen.matrix, graph_aspect_ratio(), -4.00f, 4.00f, -4.00f,
	                   4.00f, 1.00f, 2000.00f);

	//_gs_state.lights.add_light(Vector {0.00f, 0.00f, 0.00f, 1.00f}, Vector {0.00f, 0.00f, 0.00f, 1.00f}, lightsT::type::ambient);
	//_gs_state.lights.add_light(Vector {1.00f, 0.00f, -1.00f, 1.00f}, Vector {1.00f, 0.00f, 0.00f, 1.00f}, lightsT::type::directional);
	//_gs_state.lights.add_light(Vector {0.00f, 1.00f, -1.00f, 1.00f}, Vector {0.30f, 0.30f, 0.30f, 1.00f}, lightsT::type::directional);
	_gs_state.lights.add_light(Vector {-1.00f, -1.00f, -1.00f, 1.00f}, Vector {0.50f, 0.50f, 0.50f, 1.00f}, lightsT::type::directional);

	for (renderable* renderable : get_renderables())
	{
		renderable->on_gs_init();
	}
}

void add_renderable(renderable* renderable)
{
	get_renderables().push_back(renderable);
}

std::vector<renderable*>& get_transient_renderables()
{
	static std::vector<renderable*> transient_renderables;
	return transient_renderables;
}

void add_renderable_one_frame(renderable* renderable)
{
	get_transient_renderables().push_back(renderable);
}

static std::vector<std::function<void(const gs::gs_state&)>> make_renderable_funcs()
{
	std::vector<std::function<void(const gs::gs_state&)>> renderable_funcs;
	renderable_funcs.reserve(256);
	return renderable_funcs;
}

std::vector<std::function<void(const gs::gs_state&)>>& get_renderable_funcs()
{
	static std::vector<std::function<void(const gs::gs_state&)>> renderable_funcs = make_renderable_funcs();
	return renderable_funcs;
}

void add_renderable_one_frame(std::function<void(const gs::gs_state&)> func)
{
	check(get_renderable_funcs().size() < 256);

	get_renderable_funcs().push_back(func);
}

void init()
{
	printf("Initializing graphics synthesizer\n");

	// Init GIF dma channel.
	dma_channel_initialize(DMA_CHANNEL_GIF, NULL, 0);
	dma_channel_initialize(DMA_CHANNEL_VIF1, NULL, 0);
	dma_channel_fast_waits(DMA_CHANNEL_GIF);
	dma_channel_fast_waits(DMA_CHANNEL_VIF1);

	// Init the GS, framebuffer, and zbuffer.
	init_gs(frame, &z);

	// Init the drawing environment and framebuffer.
	init_drawing_environment(frame, &z);

	init_renderer(frame, &z);
}

static void draw_objects(const gs_state& gs_state)
{
	for (renderable* _renderable : get_renderables())
	{
		_renderable->render(gs_state);
	}

	for (renderable* _renderable : get_transient_renderables())
	{
		_renderable->render(gs_state);
	}

	for (std::function<void(const gs::gs_state&)>& func : get_renderable_funcs())
	{
		func(gs_state);
	}
}

static void clear_drawables()
{
	get_transient_renderables().clear();
	get_renderable_funcs().clear();
}

static int gs_render(framebuffer_t* frame, zbuffer_t* z)
{
	stats::scoped_timer render_timer(stats::scoped_timers::render);

	{
		stats::scoped_timer draw_timer(stats::scoped_timers::draw);

		clear_screen();

		// Create the world-to-view matrix.
		_gs_state.world_view = camera::get().transform.get_matrix().invert();

		_gs_state.frame   = frame;
		_gs_state.zbuffer = z;

		draw_objects(_gs_state);
	}

	// Either block until a vsync, or keep rendering until there's one
	// available.
	{
		stats::scoped_timer vsync_timer(stats::scoped_timers::render_vsync_wait);
		graph_wait_vsync();
	}

	graph_set_framebuffer_filtered(frame[context].address, frame[context].width,
	                               frame[context].psm, 0, 0);

	gs_state::flip_context();

	// We need to flip buffers outside of the chain, for some reason,
	// so we use a separate small packet.
	flip_buffers(flip_pkt, &frame[context]);

	clear_drawables();

	return 0;
}

void clear_screen()
{
	{
		packet2_inline<50> clear_packet(P2_TYPE_NORMAL, P2_MODE_NORMAL, 0);

		// Clear framebuffer but don't update zbuffer.
		clear_packet.update(draw_disable_tests, 0, &z);
		clear_packet.update(draw_clear, 0, 2048.0f - 320.0f, 2048.0f - 256.0f, frame->width, frame->height, 0x80, 0x80, 0x80);
		clear_packet.update(draw_enable_tests, 0, &z);
		clear_packet.update(draw_finish);

		// Now send our current dma chain.
		dma_wait_fast();
		dma_channel_send_packet2(clear_packet, DMA_CHANNEL_GIF, 1);
	}

	// Wait for scene to finish drawing
	draw_wait_finish();
}

void render() { gs_render(frame, &z); }

Vector gs_state::get_camera_pos() const
{
	return _gs_state.world_view.invert().get_location();
}

} // namespace gs
