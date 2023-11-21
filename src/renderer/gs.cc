#include "objects/camera.hpp"
#include "renderer/gs.hpp"
#include "renderer/renderable.hpp"
#include "stats.hpp"
#include "egg/assert.hpp"

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


namespace gs
{
static const int screen_width  = 640;
static const int screen_height = 448;

static int context = 0;

static gs_state _gs_state;

static void init_lights()
{
	// lighting
	GLfloat ambient[] = {0.2, 0.2, 0.2, 1.0};

	GLfloat l0_diffuse[] = {1.0f, 1.0f, 1.0f, 0};

	// GLfloat l1_position[] = {0, -1, 1, 0.0};
	GLfloat l1_diffuse[] = {.6f, .6f, .6f, 0.0f};

	GLfloat black[] = {0, 0, 0, 0};

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	//glEnable(GL_LIGHT1);

	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, l0_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, l0_diffuse);

	// glLightfv(GL_LIGHT1, GL_AMBIENT, black);
	// glLightfv(GL_LIGHT1, GL_DIFFUSE, l1_diffuse);
	// glLightfv(GL_LIGHT1, GL_SPECULAR, black);

	// glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 0.0f);
	// glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.005f);
	// glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.0f);
}

static void set_light_positions()
{
	GLfloat l0_position[] = {0, 1.0, 1.0, 0.0};
	GLfloat l1_position[] = {0.0, -20.0, -80.0, 1.0};

	glLightfv(GL_LIGHT0, GL_POSITION, l0_position);
	//glLightfv(GL_LIGHT1, GL_POSITION, l1_position);
}

//-----------------------------------------------------------------------------
static void gluPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar)
{
	GLdouble xmin, xmax, ymin, ymax;

	ymax = zNear * tan(fovy * M_PI / 360.0f);
	ymin = -ymax;
	xmin = ymin * aspect;
	xmax = ymax * aspect;

	glFrustum(xmin, xmax, ymin, ymax, zNear, zFar);
}

void clear_screen()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static void init_renderer()
{
	glShadeModel(GL_SMOOTH);              // Enable Smooth Shading
	glClearColor(0.5f, 0.5f, 0.5f, 0.0f); // Black Background
	glClearDepth(1.0f);                   // Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);              // Enables Depth Testing
	glEnable(GL_RESCALE_NORMAL);
	glEnable(GL_CULL_FACE);
	glDepthFunc(GL_LEQUAL); // The Type Of Depth Testing To Do
	glEnable(GL_COLOR_MATERIAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);


	glViewport(0, 0, screen_width, screen_height);

	init_lights();

	for (renderable::TIterator Itr = renderable::Itr(); Itr; ++Itr)
	{
		Itr->on_gs_init();
	}
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
	// pglAddGsMemSlot(210, 2, GS::kPsm8);

	// // 64x32
	// pglAddGsMemSlot(212, 1, GS::kPsm32);
	// pglAddGsMemSlot(213, 1, GS::kPsm32);
	// // 64x64
	// pglAddGsMemSlot(214, 2, GS::kPsm32);
	// pglAddGsMemSlot(216, 2, GS::kPsm32);
	// pglAddGsMemSlot(218, 2, GS::kPsm32);
	// pglAddGsMemSlot(220, 2, GS::kPsm32);
	// // 128x128
	// pglAddGsMemSlot(222, 8, GS::kPsm32);
	// pglAddGsMemSlot(230, 8, GS::kPsm32);
	// // 256x256
	// pglAddGsMemSlot(238, 32, GS::kPsm32);
	// pglAddGsMemSlot(270, 32, GS::kPsm32);
	// // 512x256
	// pglAddGsMemSlot(302, 64, GS::kPsm32);
	// pglAddGsMemSlot(366, 64, GS::kPsm32);

	pglPrintGsMemAllocation();
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

	// does gs memory need to be initialized?

	if (!pglHasGsMemBeenInitted())
	{
		mWarn("GS memory has not been allocated by the user; using default values.");
		initGsMemory();
	}

	init_renderer();
}

static void draw_objects(const gs_state& gs_state)
{
	// for (renderable::TIterator Itr = renderable::Itr(); Itr; ++Itr)
	// {
	// 	Itr->render(gs_state);
	// }
}

static int gs_render()
{
	static bool firstTime = true;
	{
		stats::scoped_timer draw_timer(stats::scoped_timers::draw);

		// Create the world-to-view matrix.
		_gs_state.world_view      = camera::get().transform.get_matrix().invert();
		_gs_state.camera_rotation = camera::get().transform.get_rotation();

		glMatrixMode(GL_PROJECTION); // Select The Projection Matrix
		glLoadIdentity();
		gluPerspective(camera::get().fov, (GLfloat)screen_width / (GLfloat)screen_height, 1.0f, 200000.0f);

		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(_gs_state.world_view);

		set_light_positions();

		pglBeginGeometry();

		clear_screen();

		draw_objects(_gs_state);

		glFlush();

		pglEndGeometry();

		if (!firstTime)
			pglFinishRenderingGeometry(PGL_DONT_FORCE_IMMEDIATE_STOP);
		else
			firstTime = false;
	}

	// Either block until a vsync, or keep rendering until there's one
	// available.
	{
		stats::scoped_timer vsync_timer(stats::scoped_timers::render_vsync_wait);
		pglWaitForVSync();
	}

	pglSwapBuffers();
	pglRenderGeometry();

	return 0;
}

void render() { gs_render(); }

Vector gs_state::get_camera_pos() const
{
	return _gs_state.world_view.invert().get_location();
}

Matrix gs_state::get_camera_matrix() const
{
	return _gs_state.world_view.invert();
}

Vector gs_state::get_camera_rotation() const
{
	return camera_rotation;
}

} // namespace gs
