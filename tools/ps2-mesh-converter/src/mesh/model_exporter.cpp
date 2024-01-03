#include "mesh/model_exporter.hpp"

#include <egg/mesh_header.hpp>
#include <assert.h>

static int pad_to_alignment(int current_index, int alignment = 16)
{
	int mask = alignment - 1;
	return current_index + (-current_index & mask);
}

std::vector<std::byte> serialize_meshes(uint32_t prim_type, const std::vector<MeshStrip>& strips)
{
	std::vector<std::byte> out;

	MeshFileHeader mesh_header;
	mesh_header.strips.set_num_elements(strips.size());

	size_t current_vertex_index = 0;
	std::vector<Vector> positions;
	std::vector<Vector> normals;
	std::vector<Vector2> texture_coords;
	std::vector<Vector> colors;
	std::vector<MeshTriangleStripHeader> strip_header;
	for (const MeshStrip& strip : strips)
	{
		MeshTriangleStripHeader& new_strip_header = strip_header.emplace_back();
		new_strip_header.strip_start_index        = current_vertex_index;
		new_strip_header.strip_end_index          = current_vertex_index + strip.positions.size();

		current_vertex_index += strip.positions.size();

		size_t current_pos_size = strip.positions.size();
		assert((strip.normals.size() == current_pos_size) && (strip.positions.size() == current_pos_size) && (strip.texture_coords.size() == current_pos_size));

		normals.insert(normals.end(), strip.normals.begin(), strip.normals.end());
		positions.insert(positions.end(), strip.positions.begin(), strip.positions.end());
		texture_coords.insert(texture_coords.end(), strip.texture_coords.begin(), strip.texture_coords.end());
		colors.insert(colors.end(), strip.colors.begin(), strip.colors.end());
	}

	mesh_header.pos.set(positions);
	mesh_header.nrm.set(normals);
	mesh_header.uvs.set(texture_coords);
	mesh_header.colors.set(colors);
	mesh_header.strips.set(strip_header);
	mesh_header.prim_type = prim_type;

	{
		Serializer mesh_serializer(out);
		serialize(mesh_serializer, mesh_header);
		mesh_serializer.finish_serialization();
	}

	MeshFileHeader* out_header                = (MeshFileHeader*)out.data();
	MeshTriangleStripHeader* out_strip_header = out_header->strips.get_ptr();

	printf("Testing mesh header and strip header equality...\n");
	assert(out_header->pos.num_elements() == positions.size());
	assert(out_header->nrm.num_elements() == normals.size());
	assert(out_header->uvs.num_elements() == texture_coords.size());
	assert(out_header->colors.num_elements() == colors.size());
	assert(out_header->strips.num_elements() == strips.size());

	printf("All good!\n");

	return out;
}