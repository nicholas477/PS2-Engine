#pragma once

#include <packet2.h>

struct Matrix;
struct Vector;

namespace egg::ps2::graphics
{
void init();

// Uploads a VU program
void init_vu_program(void* program_start_address, void* program_end_address);

void clear_screen(int r, int g, int b);

void wait_vsync();

void draw_mesh(const Matrix& mesh_to_screen_matrix, int num_verts, const Vector* pos);

void start_draw();
void end_draw();

packet2_t* get_curr_vif_packet();

} // namespace egg::ps2::graphics