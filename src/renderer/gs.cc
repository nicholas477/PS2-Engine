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

#include "egg-ps2-graphics-lib/egg-ps2-graphics-lib.hpp"
#include "egg-ps2-graphics-lib/gs_mem.hpp"
#include "egg-ps2-graphics-lib/draw.hpp"
#include "egg-ps2-graphics-lib/vu_programs.hpp"


namespace epg = egg::ps2::graphics;

namespace GS
{
static struct gs_options: public epg::init_options
{
	gs_options()
	    : epg::init_options()
	{
		framebuffer_width  = 640;
		framebuffer_height = 512;
		framebuffer_psm    = GS_PSM_32;
		graph_mode         = graph_get_region();
		interlaced         = true;
		ffmd               = GRAPH_MODE_FIELD;
	}
} gs_options;

IntVector2 get_screen_res()
{
	return {(int32_t)gs_options.framebuffer_width, (int32_t)gs_options.framebuffer_height};
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

		out_screen_pos.x *= 0.5f * get_screen_res().x;
		out_screen_pos.y *= 0.5f;
		out_screen_pos.y = 1.0f - out_screen_pos.y;
		out_screen_pos.y *= get_screen_res().y;

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

	epg::init(gs_options);

	// Load vertex color + textured vertex color programs
	epg::vu1_programs::get_vertex_color_program_addr()         = epg::load_vu_program(epg::vu1_programs::get_vertex_color_program_mem_address());
	epg::vu1_programs::get_vertex_color_texture_program_addr() = epg::load_vu_program(epg::vu1_programs::get_vertex_color_texture_program_mem_address());

	epg::gs_mem::allocate_texture_slot(1);
	epg::gs_mem::allocate_texture_slot(1);
	epg::gs_mem::allocate_texture_slot(1);
	epg::gs_mem::allocate_texture_slot(1);

	epg::gs_mem::allocate_texture_slot(2);
	epg::gs_mem::allocate_texture_slot(2);

	epg::gs_mem::allocate_texture_slot(4);
	epg::gs_mem::allocate_texture_slot(4);

	epg::gs_mem::allocate_texture_slot(8);
	epg::gs_mem::allocate_texture_slot(8);

	printf("Pages unallocated in gs mem (after slots): %u\n", epg::gs_mem::get_unallocated_page_num());
	epg::gs_mem::print_vram_slots();

	// Clear the screen so we're not left with whatever was left in the framebuffer
	epg::clear_screen(0, 0, 0);
	epg::wait_vsync();
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

		_gs_state.world_view   = Camera::get().transform.get_matrix().invert();
		_gs_state.view_screen  = Matrix::perspective(Camera::get().fov, get_screen_res().x, get_screen_res().y, 1.f, 5000.f);
		_gs_state.world_screen = _gs_state.world_view * _gs_state.view_screen;

		_gs_state.fog_start_end = {200.f, 2000.f};

		_gs_state.camera_rotation = Camera::get().transform.get_rotation();

		//epg::clear_screen(0xfc, 0x6c, 0x85);
		epg::clear_screen(200, 200, 200);

		{
			epg::start_draw();

			epg::set_fog_color(200, 200, 200);

			draw_objects(_gs_state);

			{
				Stats::ScopedTimer draw_timer(Stats::scoped_timers::render_finish_geom);
				epg::end_draw();
			}
		}
	}

	{
		Stats::ScopedTimer draw_timer(Stats::scoped_timers::render_vsync_wait);
		epg::wait_vsync();
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
