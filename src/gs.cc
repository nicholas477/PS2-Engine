#include <malloc.h>
#include <stdio.h>

#include "engine.h"
#include "gs.h"
#include "input.h"

#include <libgs.h>
#include <packet.h>

#include <dma_tags.h>
#include <gif_tags.h>
#include <gs_psm.h>

#include <dma.h>

#include <graph.h>

#include <draw.h>
#include <draw3d.h>

#include "mesh_data.c"

namespace gs
{
// The buffers to be used.
static framebuffer_t frame[2];
static zbuffer_t z;

static VECTOR *temp_normals;
static VECTOR *temp_lights;
static VECTOR *temp_colours;
static VECTOR *temp_vertices;

static xyz_t *xyz;
static color_t *rgbaq;

static int light_count = 4;

static const int screen_width  = 640;
static const int screen_height = 512;

static VECTOR light_direction[4] = {{0.00f, 0.00f, 0.00f, 1.00f},
                                    {1.00f, 0.00f, -1.00f, 1.00f},
                                    {0.00f, 1.00f, -1.00f, 1.00f},
                                    {-1.00f, -1.00f, -1.00f, 1.00f}};

static VECTOR light_colour[4] = {{0.00f, 0.00f, 0.00f, 1.00f},
                                 {1.00f, 0.00f, 0.00f, 1.00f},
                                 {0.30f, 0.30f, 0.30f, 1.00f},
                                 {0.50f, 0.50f, 0.50f, 1.00f}};

static int light_type[4] = {LIGHT_AMBIENT, LIGHT_DIRECTIONAL, LIGHT_DIRECTIONAL,
                            LIGHT_DIRECTIONAL};

static void init_gs(framebuffer_t *frame, zbuffer_t *z)
{
	// Define a 32-bit 640x512 framebuffer.
	frame->width  = 640;
	frame->height = 512;
	frame->mask   = 0;
	frame->psm    = GS_PSM_32;

	// Allocate some vram for our framebuffer.
	frame->address = graph_vram_allocate(frame->width, frame->height, frame->psm,
	                                     GRAPH_ALIGN_PAGE);

	frame++;

	frame->width  = 640;
	frame->height = 512;
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
}

static void init_drawing_environment(framebuffer_t *frame, zbuffer_t *z)
{
	packet_t *packet = packet_init(20, PACKET_NORMAL);

	// This is our generic qword pointer.
	qword_t *q = packet->data;

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

static void flip_buffers(packet_t *flip, framebuffer_t *frame)
{

	qword_t *q = flip->data;

	q = draw_framebuffer(q, 0, frame);
	q = draw_finish(q);

	dma_wait_fast();
	dma_channel_send_normal_ucab(DMA_CHANNEL_GIF, flip->data, q - flip->data, 0);

	draw_wait_finish();
}

qword_t *render_teapot(qword_t *q, MATRIX view_screen, VECTOR object_position,
                       VECTOR object_rotation, prim_t *prim, color_t *color,
                       framebuffer_t *frame, zbuffer_t *z)
{

	qword_t *dmatag;

	MATRIX local_world;
	MATRIX local_light;
	MATRIX world_view;

	MATRIX local_screen;

	// Now grab our qword pointer and increment past the dmatag.
	dmatag = q;
	q++;

	while (object_rotation[0] > 3.14f)
	{
		object_rotation[0] -= 6.28f;
	}
	// object_rotation[1] += 0.112f;
	while (object_rotation[1] > 3.14f)
	{
		object_rotation[1] -= 6.28f;
	}

	// Create the local_world matrix.
	create_local_world(local_world, object_position, object_rotation);

	// Create the local_light matrix.
	create_local_light(local_light, object_rotation);

	// Create the world_view matrix.
	create_world_view(world_view,
	                  const_cast<float *>(engine::_camera->get_location().vector),
	                  const_cast<float *>(engine::_camera->get_rotation().vector));

	// Create the local_screen matrix.
	create_local_screen(local_screen, local_world, world_view, view_screen);

	// Calculate the normal values.
	calculate_normals(temp_normals, vertex_count, normals, local_light);

	// Calculate the lighting values.
	calculate_lights(temp_lights, vertex_count, temp_normals, light_direction,
	                 light_colour, light_type, light_count);

	// Calculate the colour values after lighting.
	calculate_colours(temp_colours, vertex_count, colours, temp_lights);

	// Calculate the vertex values.
	calculate_vertices(temp_vertices, vertex_count, vertices, local_screen);

	// Convert floating point vertices to fixed point and translate to center of
	// screen.
	draw_convert_xyz(xyz, 2048, 2048, 32, vertex_count,
	                 (vertex_f_t *)temp_vertices);

	// Convert floating point colours to fixed point.
	draw_convert_rgbq(rgbaq, vertex_count, (vertex_f_t *)temp_vertices,
	                  (color_f_t *)temp_colours, color->a);

	// Draw the triangles using triangle primitive type.
	q = draw_prim_start(q, 0, prim, color);

	for (int i = 0; i < points_count; i++)
	{
		q->dw[0] = rgbaq[points[i]].rgbaq;
		q->dw[1] = xyz[points[i]].xyz;
		q++;
	}

	q = draw_prim_end(q, 2, DRAW_RGBAQ_REGLIST);

	// Define our dmatag for the dma chain.
	DMATAG_CNT(dmatag, q - dmatag - 1, 0, 0, 0);

	return q;
}

static int context = 0;

// Packets for doublebuffering dma sends
static packet_t *packets[2];
static packet_t *current;

// This packet is special for framebuffer switching
static packet_t *flip_pkt;

static MATRIX view_screen;
static prim_t prim;
static color_t color;

static VECTOR object_position = {0.00f, 0.00f, 0.00f, 1.00f};
static VECTOR object_rotation = {0.00f, 0.00f, 0.00f, 1.00f};

static void init_renderer(framebuffer_t *frame, zbuffer_t *z)
{
	packets[0] = packet_init(40000, PACKET_NORMAL);
	packets[1] = packet_init(40000, PACKET_NORMAL);

	// Uncached accelerated
	flip_pkt = packet_init(3, PACKET_UCAB);

	// Define the triangle primitive we want to use.
	prim.type         = PRIM_TRIANGLE;
	prim.shading      = PRIM_SHADE_GOURAUD;
	prim.mapping      = DRAW_DISABLE;
	prim.fogging      = DRAW_DISABLE;
	prim.blending     = DRAW_ENABLE;
	prim.antialiasing = DRAW_DISABLE;
	prim.mapping_type = DRAW_DISABLE;
	prim.colorfix     = PRIM_UNFIXED;

	color.r = 0x80;
	color.g = 0x80;
	color.b = 0x80;
	color.a = 0x80;
	color.q = 1.0f;

	// Allocate calculation space.
	temp_normals  = (float(*)[4])memalign(128, sizeof(VECTOR) * vertex_count);
	temp_lights   = (float(*)[4])memalign(128, sizeof(VECTOR) * vertex_count);
	temp_colours  = (float(*)[4])memalign(128, sizeof(VECTOR) * vertex_count);
	temp_vertices = (float(*)[4])memalign(128, sizeof(VECTOR) * vertex_count);

	// Allocate register space.
	xyz   = (xyz_t *)memalign(128, sizeof(u64) * vertex_count);
	rgbaq = (color_t *)memalign(128, sizeof(u64) * vertex_count);

	// Create the view_screen matrix.
	create_view_screen(view_screen, graph_aspect_ratio(), -3.00f, 3.00f, -3.00f,
	                   3.00f, 1.00f, 2000.00f);
}

void init()
{
	printf("init gs %d\n", 1);

	// Init GIF dma channel.
	dma_channel_initialize(DMA_CHANNEL_GIF, NULL, 0);
	dma_channel_fast_waits(DMA_CHANNEL_GIF);

	printf("init gs %d\n", 2);

	// Init the GS, framebuffer, and zbuffer.
	init_gs(frame, &z);

	printf("init gs %d\n", 3);

	// Init the drawing environment and framebuffer.
	init_drawing_environment(frame, &z);

	printf("init gs %d\n", 4);

	init_renderer(frame, &z);

	printf("init gs %d\n", 5);
}

static int gs_render(framebuffer_t *frame, zbuffer_t *z)
{
	current         = packets[context];
	qword_t *q      = current->data;
	qword_t *dmatag = q;
	q++;

	// Clear framebuffer without any pixel testing.
	q = draw_disable_tests(q, 0, z);
	q = draw_clear(q, 0, 2048.0f - 320.0f, 2048.0f - 256.0f, frame->width,
	               frame->height, 0x80, 0x80, 0x80);
	q = draw_enable_tests(q, 0, z);

	DMATAG_CNT(dmatag, q - dmatag - 1, 0, 0, 0);

	// render teapots
	color.a            = 0x40;
	object_position[0] = 30.0f;
	q                  = render_teapot(q, view_screen, object_position, object_rotation, &prim,
                      &color, frame, z);

	object_position[0] = -30.0f;
	q                  = render_teapot(q, view_screen, object_position, object_rotation, &prim,
                      &color, frame, z);

	color.a            = 0x80;
	object_position[0] = 0.0f;
	object_position[1] = -20.0f;
	q                  = render_teapot(q, view_screen, object_position, object_rotation, &prim,
                      &color, frame, z);

	object_position[1] = 20.0f;
	q                  = render_teapot(q, view_screen, object_position, object_rotation, &prim,
                      &color, frame, z);

	object_position[0] = 0.0f;
	object_position[1] = 0.0f;

	dmatag = q;
	q++;

	q = draw_finish(q);

	DMATAG_END(dmatag, q - dmatag - 1, 0, 0, 0);

	// Now send our current dma chain.
	dma_wait_fast();
	dma_channel_send_chain(DMA_CHANNEL_GIF, current->data, q - current->data, 0,
	                       0);

	// Either block until a vsync, or keep rendering until there's one
	// available.
	graph_wait_vsync();

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