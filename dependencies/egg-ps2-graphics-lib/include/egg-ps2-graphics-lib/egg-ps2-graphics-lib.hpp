#pragma once

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
void end_draw();
} // namespace egg::ps2::graphics