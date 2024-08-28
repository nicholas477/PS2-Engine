#pragma once

#include <tamtypes.h>

#include "egg/math_types.hpp"

namespace egg::ps2::graphics
{
struct texture_descriptor;

struct mesh_descriptor
{
	mesh_descriptor();

	Vector* pos;
	Vector* color;
	Vector* uvs;

	texture_descriptor* texture;

	// Has to be at least 3
	u32 num_verts;

	// Address of the VU program loaded in memory used to perform vertex
	// processing on this mesh.
	//
	// Note: this is a vu1 memory address! Valid values for a vu1 memory address
	// are 0-1023
	s32 vu_program_addr;

	Vector screen_scale;

	// Fog settings
	bool enable_fog;

	// The start offset of the fog
	float fog_offset;

	// fog distance scaling
	float fog_scale;

public:
	bool is_valid(bool print_why_invalid = true) const;

	void set_fog_start_and_end(float fog_start, float fog_end);
};

void draw_mesh_strip(const Matrix& mesh_to_screen_matrix, const mesh_descriptor& mesh);
} // namespace egg::ps2::graphics