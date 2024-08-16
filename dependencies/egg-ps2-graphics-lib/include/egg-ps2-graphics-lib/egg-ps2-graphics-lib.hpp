#pragma once

#include <packet2.h>

struct Matrix;
struct Vector;

namespace egg::ps2::graphics
{
void init();

// Uploads a VU program, returns the address of the loaded program
u32 load_vu_program(void* program_start_address, void* program_end_address);

void clear_screen(int r, int g, int b);

void wait_vsync();

void start_draw();
void end_draw();
} // namespace egg::ps2::graphics