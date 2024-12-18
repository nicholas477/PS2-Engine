#include "egg-ps2-graphics-lib/vu_programs.hpp"

#include "egg-ps2-graphics-lib/egg-ps2-graphics-lib.hpp"

namespace egg::ps2::graphics
{
void vu_program::load_program()
{
	program_address = load_vu_program(address.first, address.second);
	is_loaded       = true;
}

namespace vu1_programs
{
DEFINE_VU_PROGRAM(xgkick, xgkick);
DEFINE_VU_PROGRAM(vertex_color_texture_renderer, VertexColorTextureRenderer);
DEFINE_VU_PROGRAM(vertex_color_renderer, VertexColorRenderer);
DEFINE_VU_PROGRAM(project, Project);
DEFINE_VU_PROGRAM(project_clip, ProjectClip);
DEFINE_VU_PROGRAM(vertex_color, VertexColor);
DEFINE_VU_PROGRAM(jump_table, JumpTable);
} // namespace vu1_programs
} // namespace egg::ps2::graphics