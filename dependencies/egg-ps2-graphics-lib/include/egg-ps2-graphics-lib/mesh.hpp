#pragma once

#include <tamtypes.h>

struct Matrix;
struct Vector;

namespace egg::ps2::graphics
{
struct mesh_descriptor
{
	Vector* pos   = nullptr;
	Vector* color = nullptr;

	// Has to be at least 3
	u32 num_verts = 0;

	// Address of the VU program loaded in memory used to perform vertex
	// processing on this mesh.
	//
	// Note: this is a vu1 memory address! Valid values for a vu1 memory address
	// are 0-1023
	u32 vu_program_addr = 0;

	bool is_valid(bool print_why_invalid = true) const;
};

void draw_mesh(const Matrix& mesh_to_screen_matrix, const mesh_descriptor& mesh);
} // namespace egg::ps2::graphics