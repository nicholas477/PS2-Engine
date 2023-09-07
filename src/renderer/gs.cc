#include <malloc.h>
#include <stdio.h>
#include <vector>

#include "objects/camera.hpp"
#include "renderer/gs.hpp"
#include "input.hpp"
#include "renderer/renderable.hpp"
#include "stats.hpp"

#include <libgs.h>
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
	graph_initialize(frame->address, frame->width, frame->height, frame->psm, 0,
	                 0);

	get_path1();
}

static void init_drawing_environment(framebuffer_t* frame, zbuffer_t* z)
{
	packet_t* packet = packet_init(20, PACKET_NORMAL);

	// This is our generic qword pointer.
	qword_t* q = packet->data;

	// This will setup a default drawing environment.
	q = draw_setup_environment(q, 0, frame, z);

	// Now reset the primitive origin to 2048-width/2,2048-height/2.
	q = draw_primitive_xyoffset(q, 0, (2048 - 320), (2048 - 256));

	// Finish setting up the environment.
	q = draw_finish(q);

	// Now send the packet, no need to wait since it's the first.
	dma_channel_send_normal(DMA_CHANNEL_GIF, packet->data, q - packet->data, 0,
	                        0);
	dma_wait_fast();

	free(packet);
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
static packet_t* packets[2];
static packet_t* current;

// This packet is special for framebuffer switching
static packet_t* flip_pkt;

static gs_state _gs_state;

static void init_renderer(framebuffer_t* frame, zbuffer_t* z)
{
	packets[0] = packet_init(160000, PACKET_NORMAL);
	packets[1] = packet_init(160000, PACKET_NORMAL);

	// Uncached accelerated
	flip_pkt = packet_init(3, PACKET_UCAB);

	// Create the view_screen matrix.
	create_view_screen(_gs_state.view_screen.matrix, graph_aspect_ratio(), -4.00f, 4.00f, -4.00f,
	                   4.00f, 1.00f, 2000.00f);

	//_gs_state.lights.add_light(Vector {0.00f, 0.00f, 0.00f, 1.00f}, Vector {0.00f, 0.00f, 0.00f, 1.00f}, lightsT::type::ambient);
	//_gs_state.lights.add_light(Vector {1.00f, 0.00f, -1.00f, 1.00f}, Vector {1.00f, 0.00f, 0.00f, 1.00f}, lightsT::type::directional);
	//_gs_state.lights.add_light(Vector {0.00f, 1.00f, -1.00f, 1.00f}, Vector {0.30f, 0.30f, 0.30f, 1.00f}, lightsT::type::directional);
	_gs_state.lights.add_light(Vector {-1.00f, -1.00f, -1.00f, 1.00f}, Vector {0.50f, 0.50f, 0.50f, 1.00f}, lightsT::type::directional);
}

std::vector<renderable*>& get_renderables()
{
	static std::vector<renderable*> renderables;
	return renderables;
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

std::vector<std::function<qword_t*(qword_t*, const gs::gs_state&)>>& get_renderable_lambdas()
{
	static std::vector<std::function<qword_t*(qword_t*, const gs::gs_state&)>> transient_renderable_funcs;
	return transient_renderable_funcs;
}
void add_renderable_lambda_one_frame(std::function<qword_t*(qword_t*, const gs::gs_state&)>&& func)
{
	get_renderable_lambdas().emplace_back(func);
}

void init()
{
	printf("Initializing graphics synthesizer\n");

	// Init GIF dma channel.
	dma_channel_initialize(DMA_CHANNEL_GIF, NULL, 0);
	dma_channel_fast_waits(DMA_CHANNEL_GIF);

	// Init the GS, framebuffer, and zbuffer.
	init_gs(frame, &z);

	// Init the drawing environment and framebuffer.
	init_drawing_environment(frame, &z);

	init_renderer(frame, &z);
}

static qword_t* draw_objects(qword_t* q, const gs_state& gs_state)
{
	for (renderable* _renderable : get_renderables())
	{
		q = _renderable->render(q, gs_state);
	}

	for (renderable* _renderable : get_transient_renderables())
	{
		q = _renderable->render(q, gs_state);
	}
	get_transient_renderables().clear();

	for (std::function<qword_t*(qword_t*, const gs::gs_state&)>& lambda : get_renderable_lambdas())
	{
		q = lambda(q, gs_state);
	}
	get_renderable_lambdas().clear();

	return q;
}

static int gs_render(framebuffer_t* frame, zbuffer_t* z)
{
	stats::scoped_timer render_timer(stats::scoped_timers::render);

	current         = packets[context];
	qword_t* q      = current->data;
	qword_t* dmatag = q;
	q++;

	{
		stats::scoped_timer draw_timer(stats::scoped_timers::draw);

		// Clear framebuffer without any pixel testing.
		q = draw_disable_tests(q, 0, z);
		q = draw_clear(q, 0, 2048.0f - 320.0f, 2048.0f - 256.0f, frame->width,
		               frame->height, 0x80, 0x80, 0x80);
		q = draw_enable_tests(q, 0, z);

		DMATAG_CNT(dmatag, q - dmatag - 1, 0, 0, 0);

		// Create the world_view matrix.
		_gs_state.view_world = camera::get().transform.get_matrix().invert();

		_gs_state.frame   = frame;
		_gs_state.zbuffer = z;

		q = draw_objects(q, _gs_state);

		dmatag = q;
		q++;

		q = draw_finish(q);

		DMATAG_END(dmatag, q - dmatag - 1, 0, 0, 0);
	}

	// Now send our current dma chain.
	dma_wait_fast();
	dma_channel_send_chain(DMA_CHANNEL_GIF, current->data, q - current->data, 0,
	                       0);

	// Either block until a vsync, or keep rendering until there's one
	// available.
	{
		stats::scoped_timer draw_timer(stats::scoped_timers::render_vsync_wait);
		graph_wait_vsync();
	}

	draw_wait_finish();
	graph_set_framebuffer_filtered(frame[context].address, frame[context].width,
	                               frame[context].psm, 0, 0);

	// Switch context.
	context ^= 1;

	// We need to flip buffers outside of the chain, for some reason,
	// so we use a separate small packet.
	flip_buffers(flip_pkt, &frame[context]);

	return 0;
}

void render() { gs_render(frame, &z); }
} // namespace gs
