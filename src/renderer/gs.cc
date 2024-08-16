#include "objects/camera.hpp"
#include "renderer/gs.hpp"
#include "renderer/renderable.hpp"
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

#include "renderer/text.hpp"
#include "renderer/vu1/vertex_color_program.hpp"

#include "egg-ps2-graphics-lib/egg-ps2-graphics-lib.hpp"


namespace GS
{
static const int screen_width  = 640;
static const int screen_height = 512;

IntVector2 get_screen_res()
{
	return {screen_width, screen_height};
}

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

static bool has_initialized = false;

bool has_gs_initialized()
{
	return has_initialized;
}

static void rendering_finished()
{
	//printf("rendering finished!!!!!!!!!!!!!!!\n");
}

void init()
{
	printf("Initializing graphics synthesizer\n");

	egg::ps2::graphics::init();

	egg::ps2::graphics::load_vu_program(mVsmStartAddr(VertexColorRenderer), mVsmEndAddr(VertexColorRenderer));
}

static void draw_objects(const GSState& gs_state)
{
	int i = 0;
	for (Renderable::TIterator Itr = Renderable::Itr(); Itr; ++Itr)
	{
		//Debuggable::print_debug_object(&*Itr);
		Itr->render(gs_state);
		i++;
	}

	//printf("rendered %d objects\n", i);
} // namespace GS

void render()
{
	if (Engine::get_frame_counter() % 60 == 0)
	{
		printf("Rendering frame %d\n", Engine::get_frame_counter());
	}

	{
		Stats::ScopedTimer draw_timer(Stats::scoped_timers::draw);

		_gs_state.world_view = Camera::get().transform.get_matrix().invert() * Matrix::from_scale(Vector(1.f, 1.f, 1.f));
		//_gs_state.view_screen = Matrix::perspective(Camera::get().fov, (GLfloat)screen_width / (GLfloat)screen_height, 1.f, 2000.f);
		_gs_state.view_screen     = Matrix::frustum(-3.00f, 3.00f, 3.00f, -3.00f, 1.00f, 2000.f);
		_gs_state.world_screen    = _gs_state.world_view * _gs_state.view_screen;
		_gs_state.camera_rotation = Camera::get().transform.get_rotation();

		egg::ps2::graphics::clear_screen(0x40, 0x40, 0x40);

		egg::ps2::graphics::start_draw();

		draw_objects(_gs_state);

		egg::ps2::graphics::end_draw();
	}

	{
		Stats::ScopedTimer draw_timer(Stats::scoped_timers::render_vsync_wait);
		egg::ps2::graphics::wait_vsync();
	}
}

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
