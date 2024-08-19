#pragma once

#include <packet2.h>
#include <array>
#include "types.hpp"

struct Matrix;
struct Vector;

namespace egg::ps2::graphics
{
using vif_packet_t = utils::inline_packet2<1024>;

void init();

// Uploads a VU program, returns the address of the loaded program
u32 load_vu_program(void* program_start_address, void* program_end_address);

void clear_screen(int r, int g, int b);

void wait_vsync();

void start_draw();
void end_draw();

// Returns the two vif packets in use
std::array<vif_packet_t, 2>& get_vif_packets();
packet2_t* get_current_vif_packet();
u8 get_vif_packet_context();
void flip_vip_packet_context();
} // namespace egg::ps2::graphics