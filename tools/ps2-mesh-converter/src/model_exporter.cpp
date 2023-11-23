#include "model_exporter.hpp"

#include <egg/mesh_header.hpp>

static int pad_to_alignment(int current_index, int alignment = 16)
{
	int mask = alignment - 1;
	return current_index + (-current_index & mask);
}

std::vector<std::byte> serialize_mesh(const std::vector<Vector>& positions,
                                      const std::vector<Vector>& normals,
                                      const std::vector<Vector2>& texture_coords,
                                      const std::vector<Vector>& colors)
{
	std::vector<std::byte> out;

	MeshFileHeader mesh_header;
	mesh_header.nrm.set_num_elements(normals.size());
	mesh_header.pos.set_num_elements(positions.size());
	mesh_header.uvs.set_num_elements(texture_coords.size());
	mesh_header.colors.set_num_elements(colors.size());
	mesh_header.strips.set_num_elements(1);

	MeshTriangleStripHeader strip_header;
	strip_header.strip_start_index = 0;
	strip_header.strip_end_index   = positions.size();

	int current_byte_index = 0; // Index of the byte currently being written to

	// Calculate header offsets and the size of the file
	{
		current_byte_index += sizeof(mesh_header);

		mesh_header.strips.offset = current_byte_index - offsetof(MeshFileHeader, strips);
		current_byte_index += sizeof(strip_header);

		current_byte_index     = pad_to_alignment(current_byte_index, 16);
		mesh_header.pos.offset = current_byte_index - offsetof(MeshFileHeader, pos);
		current_byte_index += positions.size() * sizeof(Vector);

		current_byte_index     = pad_to_alignment(current_byte_index, 16);
		mesh_header.nrm.offset = current_byte_index - offsetof(MeshFileHeader, nrm);
		current_byte_index += normals.size() * sizeof(Vector);

		current_byte_index     = pad_to_alignment(current_byte_index, 16);
		mesh_header.uvs.offset = current_byte_index - offsetof(MeshFileHeader, uvs);
		current_byte_index += texture_coords.size() * sizeof(Vector2);

		current_byte_index        = pad_to_alignment(current_byte_index, 16);
		mesh_header.colors.offset = current_byte_index - offsetof(MeshFileHeader, colors);
		current_byte_index += colors.size() * sizeof(Vector);
	}

	out.resize(current_byte_index);

	// Now copy over the data
	memcpy(out.data(), &mesh_header, sizeof(mesh_header));
	memcpy(out.data() + mesh_header.strips.offset + offsetof(MeshFileHeader, strips), &strip_header, sizeof(strip_header));
	memcpy(out.data() + mesh_header.pos.offset + offsetof(MeshFileHeader, pos), positions.data(), positions.size() * sizeof(Vector));
	memcpy(out.data() + mesh_header.nrm.offset + offsetof(MeshFileHeader, nrm), normals.data(), normals.size() * sizeof(Vector));
	memcpy(out.data() + mesh_header.uvs.offset + offsetof(MeshFileHeader, uvs), texture_coords.data(), texture_coords.size() * sizeof(Vector2));
	memcpy(out.data() + mesh_header.colors.offset + offsetof(MeshFileHeader, colors), colors.data(), colors.size() * sizeof(Vector));

	MeshFileHeader* out_header                = (MeshFileHeader*)out.data();
	MeshTriangleStripHeader* out_strip_header = out_header->strips.get_ptr();

	printf("Testing mesh header and strip header equality...\n");
	assert(memcmp(&mesh_header, out_header, sizeof(mesh_header)) == 0);
	assert(memcmp(&strip_header, out_strip_header, sizeof(strip_header)) == 0);

	printf("Testing position equality...\n");
	assert(positions.size() == out_header->pos.num_elements());
	assert(memcmp(positions.data(), out_header->pos.get_ptr(), out_header->pos.length) == 0);

	printf("Testing normals equality...\n");
	assert(normals.size() == out_header->nrm.num_elements());
	assert(memcmp(normals.data(), out_header->nrm.get_ptr(), out_header->nrm.length) == 0);

	printf("Texture coord equality...\n");
	assert(texture_coords.size() == out_header->uvs.num_elements());
	assert(memcmp(texture_coords.data(), out_header->uvs.get_ptr(), out_header->uvs.length) == 0);

	printf("Testing normals equality...\n");
	assert(colors.size() == out_header->colors.num_elements());
	assert(memcmp(colors.data(), out_header->colors.get_ptr(), out_header->colors.length) == 0);

	printf("All good!\n");

	return out;
}