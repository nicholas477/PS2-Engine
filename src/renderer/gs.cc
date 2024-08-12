#include "objects/camera.hpp"
#include "renderer/gs.hpp"
#include "renderer/renderable.hpp"
#include "renderer/ps2gl_renderers/vertex_color_renderer.hpp"
#include "renderer/ps2gl_renderers/vertex_color_vegetation_renderer.hpp"
#include "stats.hpp"
#include "egg/assert.hpp"
#include "utils/debuggable.hpp"
#include "threading.hpp"

/* libc */
#include <stdio.h>
#include <stdlib.h>

/* ps2sdk */
#include "kernel.h"
#include "graph.h"

/* GL */
#include "GL/ps2gl.h"

/* ps2stuff */
#include "ps2s/core.h"
#include "ps2s/displayenv.h"
#include "ps2s/drawenv.h"
#include "ps2s/gs.h"
#include "ps2s/timer.h"

/* ps2gl */
#include "ps2gl/debug.h"
#include "ps2gl/displaycontext.h"
#include "ps2gl/drawcontext.h"
#include "ps2gl/glcontext.h"

#include <GL/glut.h>         // The GL Utility Toolkit (Glut) Header
#include <ps2gl/glcontext.h> // The GL Utility Toolkit (Glut) Header
#include <ps2gl/matrix.h>    // The GL Utility Toolkit (Glut) Header

#include "renderer/text.hpp"


namespace GS
{
static const int screen_width  = 640;
static const int screen_height = 448;

IntVector2 get_screen_res()
{
	return {screen_width, screen_height};
}

static int context = 0;

static GSState _gs_state;

bool GSState::world_to_screen(const Vector& world_vec, Vector& out_screen_pos) const
{
	Vector location = world_vec;
	location.w      = 1.f;

	out_screen_pos = (world_view * view_screen).transform_vector(location);

	if (out_screen_pos.w > 0.f)
	{
		out_screen_pos.x /= out_screen_pos.w;
		out_screen_pos.y /= out_screen_pos.w;
		out_screen_pos.z /= out_screen_pos.w;

		out_screen_pos.x += 1.0f;
		out_screen_pos.y += 1.0f;

		out_screen_pos.x *= 0.5f * screen_width;
		out_screen_pos.y *= 0.5f;
		out_screen_pos.y = 1.0f - out_screen_pos.y;
		out_screen_pos.y *= screen_height;

		return true;
	}
	return false;
}

static void init_lights()
{
	// lighting
	GLfloat ambient[]    = {0.0, 0.0, 0.0, 0.0};
	GLfloat l0_diffuse[] = {1.0f, 1.0f, 1.0f, 0};


	GLfloat black[] = {0, 0, 0, 0};

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
}

void clear_screen()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static bool has_initialized = false;

static void init_renderer()
{
	printf("Initializing graphics synthesizer: renderer\n");
	glShadeModel(GL_SMOOTH);              // Enable Smooth Shading
	glClearColor(0.5f, 0.5f, 0.5f, 0.5f); // Black Background
	glEnable(GL_DEPTH_TEST);              // Enables Depth Testing
	glDepthFunc(GL_LEQUAL);               // The Type Of Depth Testing To Do
	pglEnable(PGL_CLIPPING);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glDisableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	//init_lights();

	tsLoadFont();

	CVertexColorRenderer::Register();
	CVertexColorVegetationRenderer::Register();

	has_initialized = true;

	printf("Initializing graphics synthesizer: calling on_gs_init\n");
	for (Renderable::TIterator Itr = Renderable::Itr(); Itr; ++Itr)
	{
		Itr->on_gs_init();
	}

	printf("Succesfully initialized GS\n");
}

bool has_gs_initialized()
{
	return has_initialized;
}

static void
initGsMemory()
{
	// frame and depth buffer
	pgl_slot_handle_t frame_slot_0, frame_slot_1, depth_slot;
	frame_slot_0 = pglAddGsMemSlot(0, 70, GS::kPsm32);
	frame_slot_1 = pglAddGsMemSlot(70, 70, GS::kPsm32);
	depth_slot   = pglAddGsMemSlot(140, 70, GS::kPsmz24);
	// lock these slots so that they aren't allocated by the memory manager
	pglLockGsMemSlot(frame_slot_0);
	pglLockGsMemSlot(frame_slot_1);
	pglLockGsMemSlot(depth_slot);

	// create gs memory area objects to use for frame and depth buffers
	pgl_area_handle_t frame_area_0, frame_area_1, depth_area;
	frame_area_0 = pglCreateGsMemArea(screen_width, screen_height / 2, GS::kPsm24);
	frame_area_1 = pglCreateGsMemArea(screen_width, screen_height / 2, GS::kPsm24);
	depth_area   = pglCreateGsMemArea(screen_width, screen_height / 2, GS::kPsmz24);

	// bind the areas to the slots we created above
	pglBindGsMemAreaToSlot(frame_area_0, frame_slot_0);
	pglBindGsMemAreaToSlot(frame_area_1, frame_slot_1);
	pglBindGsMemAreaToSlot(depth_area, depth_slot);

	// draw to the new areas...
	pglSetDrawBuffers(PGL_INTERLACED, frame_area_0, frame_area_1, depth_area);
	// ...and display from them
	pglSetDisplayBuffers(PGL_INTERLACED, frame_area_0, frame_area_1);

	// 32 bit

	// a slot for fonts (probably)
	pglAddGsMemSlot(210, 4, GS::kPsm8);

	// 64x32
	pglAddGsMemSlot(214, 1, GS::kPsm32);
	pglAddGsMemSlot(215, 1, GS::kPsm32);
	// 64x64
	pglAddGsMemSlot(216, 2, GS::kPsm32);
	pglAddGsMemSlot(218, 2, GS::kPsm32);
	pglAddGsMemSlot(220, 2, GS::kPsm32);
	pglAddGsMemSlot(222, 2, GS::kPsm32);
	// 128x128
	pglAddGsMemSlot(224, 8, GS::kPsm32);
	pglAddGsMemSlot(232, 8, GS::kPsm32);
	// 256x256
	pglAddGsMemSlot(240, 32, GS::kPsm32);
	pglAddGsMemSlot(272, 32, GS::kPsm32);
	// 512x256
	pglAddGsMemSlot(302, 64, GS::kPsm32);
	pglAddGsMemSlot(366, 64, GS::kPsm32);

	pglPrintGsMemAllocation();
}

static void rendering_finished()
{
	//printf("rendering finished!!!!!!!!!!!!!!!\n");
}

void init()
{
	printf("Initializing graphics synthesizer\n");

	if (!pglHasLibraryBeenInitted())
	{
		// reset the machine
		//      sceDevVif0Reset();
		//      sceDevVu0Reset();
		//      sceDmaReset(1);
		//      sceGsResetPath();

		// Reset the GIF. OSDSYS leaves PATH3 busy, that ends up having
		// our PATH1/2 transfers ignored by the GIF.
		*GIF::Registers::ctrl = 1;

		//      sceGsResetGraph(0, SCE_GS_INTERLACE, SCE_GS_NTSC, SCE_GS_FRAME);
		SetGsCrt(1 /* non-interlaced */, 2 /* ntsc */, 1 /* frame */);

		mWarn("ps2gl library has not been initialized by the user; using default values.");
		int immBufferVertexSize = 64 * 1024;
		pglInit(immBufferVertexSize, 1000);
	}

	pglSetRenderingFinishedCallback(rendering_finished);

	// does gs memory need to be initialized?

	if (!pglHasGsMemBeenInitted())
	{
		mWarn("GS memory has not been allocated by the user; using default values.");
		initGsMemory();
	}

	init_renderer();
}

static void draw_objects(const GSState& gs_state)
{
	int i = 0;
	for (Renderable::TIterator Itr = Renderable::Itr(); Itr; ++Itr)
	{
		//Debuggable::print_debug_object(&*Itr);
		Itr->render(gs_state);
	}

	// Text rendering
	for (TextRenderable::TIterator Itr = TextRenderable::Itr(); Itr; ++Itr)
	{
		Itr->render(gs_state);
	}
} // namespace GS

static void gs_render()
{
	static bool firstTime = true;
	{
		Stats::ScopedTimer draw_timer(Stats::scoped_timers::draw);

		constexpr float world_scale = 0.0001f;
		// Create the world-to-view matrix.
		_gs_state.world_view      = Camera::get().transform.get_matrix().invert();
		_gs_state.view_screen     = Matrix::perspective(Camera::get().fov, (GLfloat)screen_width / (GLfloat)screen_height, 0.001f, 1.f);
		_gs_state.camera_rotation = Camera::get().transform.get_rotation();

		glMatrixMode(GL_PROJECTION); // Select The Projection Matrix
		glLoadIdentity();
		glMultMatrixf(_gs_state.view_screen);
		glMultMatrixf(_gs_state.world_view * Matrix::from_scale(Vector(world_scale, world_scale, world_scale)));

		glMatrixMode(GL_MODELVIEW);

		pglBeginGeometry();

		clear_screen();

		draw_objects(_gs_state);

		{
			Stats::ScopedTimer flush_timer(Stats::scoped_timers::render_flush);
			glFlush();
		}

		pglEndGeometry();
		if (!firstTime)
		{
			Stats::ScopedTimer finish_geo_timer(Stats::scoped_timers::render_finish_geom);
			pglFinishRenderingGeometry(PGL_DONT_FORCE_IMMEDIATE_STOP);
		}
		else
		{
			firstTime = false;
		}
	}

	// Either block until a vsync, or keep rendering until there's one
	// available.
	{
		Stats::ScopedTimer vsync_timer(Stats::scoped_timers::render_vsync_wait);
		pglWaitForVSync();
	}

	pglSwapBuffers();
	pglRenderGeometry();
}

void render() { gs_render(); }

Vector GSState::get_camera_pos() const
{
	return _gs_state.world_view.invert().get_location();
}

Matrix GSState::get_camera_matrix() const
{
	return _gs_state.world_view.invert();
}

Vector GSState::get_camera_rotation() const
{
	return camera_rotation;
}

} // namespace GS
