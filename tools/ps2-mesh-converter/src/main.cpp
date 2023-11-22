
#include <stdio.h>
#include <assert.h>
#include <fstream>
#include <iterator>
#include <egg/math_types.hpp>
#include <cstddef>
#include <algorithm>
#include "types.hpp"

#include <cxxopts.hpp>

#define FAST_OBJ_IMPLEMENTATION
#include "../extern/fast_obj.h"

#include "meshoptimizer.h"

const size_t kCacheSize = 16;

static Mesh parseObj(const char* path)
{
	fastObjMesh* obj = fast_obj_read(path);
	if (!obj)
	{
		printf("Error loading %s: file not found\n", path);
		return Mesh();
	}

	size_t total_indices = 0;

	for (unsigned int i = 0; i < obj->face_count; ++i)
		total_indices += 3 * (obj->face_vertices[i] - 2);

	std::vector<Vertex> vertices(total_indices);

	size_t vertex_offset = 0;
	size_t index_offset  = 0;

	for (unsigned int i = 0; i < obj->face_count; ++i)
	{
		for (unsigned int j = 0; j < obj->face_vertices[i]; ++j)
		{
			fastObjIndex gi = obj->indices[index_offset + j];

			Vertex v =
			    {
			        obj->positions[gi.p * 3 + 0],
			        obj->positions[gi.p * 3 + 1],
			        obj->positions[gi.p * 3 + 2],
			        obj->normals[gi.n * 3 + 0],
			        obj->normals[gi.n * 3 + 1],
			        obj->normals[gi.n * 3 + 2],
			        obj->texcoords[gi.t * 2 + 0],
			        obj->texcoords[gi.t * 2 + 1],
			        // obj->colors[gi.p * 3 + 0],
			        // obj->colors[gi.p * 3 + 1],
			        // obj->colors[gi.p * 3 + 2],
			    };

			// triangulate polygon on the fly; offset-3 is always the first polygon vertex
			if (j >= 3)
			{
				vertices[vertex_offset + 0] = vertices[vertex_offset - 3];
				vertices[vertex_offset + 1] = vertices[vertex_offset - 1];
				vertex_offset += 2;
			}

			vertices[vertex_offset] = v;
			vertex_offset++;
		}

		index_offset += obj->face_vertices[i];
	}

	fast_obj_destroy(obj);

	Mesh result;

	std::vector<unsigned int> remap(total_indices);

	size_t total_vertices = meshopt_generateVertexRemap(&remap[0], NULL, total_indices, &vertices[0], total_indices, sizeof(Vertex));

	result.indices.resize(total_indices);
	meshopt_remapIndexBuffer(&result.indices[0], NULL, total_indices, &remap[0]);

	result.vertices.resize(total_vertices);
	meshopt_remapVertexBuffer(&result.vertices[0], &vertices[0], total_indices, sizeof(Vertex), &remap[0]);

	return result;
}

static bool loadMesh(Mesh& mesh, const char* path)
{
	mesh = parseObj(path);

	if (mesh.vertices.empty())
	{
		printf("Mesh %s is empty, skipping\n", path);
		return false;
	}

	return true;
}

static bool rotateTriangle(Triangle& t)
{
	int c01 = memcmp(&t.v[0], &t.v[1], sizeof(Vertex));
	int c02 = memcmp(&t.v[0], &t.v[2], sizeof(Vertex));
	int c12 = memcmp(&t.v[1], &t.v[2], sizeof(Vertex));

	if (c12 < 0 && c01 > 0)
	{
		// 1 is minimum, rotate 012 => 120
		Vertex tv = t.v[0];
		t.v[0] = t.v[1], t.v[1] = t.v[2], t.v[2] = tv;
	}
	else if (c02 > 0 && c12 > 0)
	{
		// 2 is minimum, rotate 012 => 201
		Vertex tv = t.v[2];
		t.v[2] = t.v[1], t.v[1] = t.v[0], t.v[0] = tv;
	}

	return c01 != 0 && c02 != 0 && c12 != 0;
}

static unsigned int hashRange(const char* key, size_t len)
{
	// MurmurHash2
	const unsigned int m = 0x5bd1e995;
	const int r          = 24;

	unsigned int h = 0;

	while (len >= 4)
	{
		unsigned int k = *reinterpret_cast<const unsigned int*>(key);

		k *= m;
		k ^= k >> r;
		k *= m;

		h *= m;
		h ^= k;

		key += 4;
		len -= 4;
	}

	return h;
}

static unsigned int hashMesh(const Mesh& mesh)
{
	size_t triangle_count = mesh.indices.size() / 3;

	const Vertex* vertices      = &mesh.vertices[0];
	const unsigned int* indices = &mesh.indices[0];

	unsigned int h1 = 0;
	unsigned int h2 = 0;

	for (size_t i = 0; i < triangle_count; ++i)
	{
		Triangle t;
		t.v[0] = vertices[indices[i * 3 + 0]];
		t.v[1] = vertices[indices[i * 3 + 1]];
		t.v[2] = vertices[indices[i * 3 + 2]];

		// skip degenerate triangles since some algorithms don't preserve them
		if (rotateTriangle(t))
		{
			unsigned int hash = hashRange(t.data, sizeof(t.data));

			h1 ^= hash;
			h2 += hash;
		}
	}

	return h1 * 0x5bd1e995 + h2;
}

static std::vector<unsigned int> stripify(const Mesh& mesh, bool use_restart, char desc)
{
	unsigned int restart_index = use_restart ? ~0u : 0;

	// note: input mesh is assumed to be optimized for vertex cache and vertex fetch
	std::vector<unsigned int> strip(meshopt_stripifyBound(mesh.indices.size()));
	strip.resize(meshopt_stripify(&strip[0], &mesh.indices[0], mesh.indices.size(), mesh.vertices.size(), restart_index));

	Mesh copy = mesh;
	copy.indices.resize(meshopt_unstripify(&copy.indices[0], &strip[0], strip.size(), restart_index));
	assert(copy.indices.size() <= meshopt_unstripifyBound(strip.size()));

	assert(isMeshValid(copy));
	assert(hashMesh(mesh) == hashMesh(copy));

	return strip;
}

static int pad_to_alignment(int current_index, int alignment = 16)
{
	int mask = alignment - 1;
	return current_index + (-current_index & mask);
}

static std::vector<std::byte> serialize_mesh(const std::vector<Vector>& positions,
                                             const std::vector<Vector>& normals,
                                             const std::vector<Vector2>& texture_coords)
{
	std::vector<std::byte> out;

	MeshFileHeader mesh_header;
	mesh_header.nrm.set_num_elements(normals.size());
	mesh_header.pos.set_num_elements(positions.size());
	mesh_header.uvs.set_num_elements(texture_coords.size());
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
		//mesh_header.uvs.offset = -1;
		current_byte_index += texture_coords.size() * sizeof(Vector2);
	}

	out.resize(current_byte_index);

	// Now copy over the data
	memcpy(out.data(), &mesh_header, sizeof(mesh_header));
	memcpy(out.data() + mesh_header.strips.offset + offsetof(MeshFileHeader, strips), &strip_header, sizeof(strip_header));
	memcpy(out.data() + mesh_header.pos.offset + offsetof(MeshFileHeader, pos), positions.data(), positions.size() * sizeof(Vector));
	memcpy(out.data() + mesh_header.nrm.offset + offsetof(MeshFileHeader, nrm), normals.data(), normals.size() * sizeof(Vector));
	memcpy(out.data() + mesh_header.uvs.offset + offsetof(MeshFileHeader, uvs), texture_coords.data(), texture_coords.size() * sizeof(Vector2));


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

	printf("All good!\n");

	return out;
}

static std::string input_path;
static std::string output_path;
static bool write_output = true;

static void process()
{
	printf("Processing model file: %s\n", input_path.c_str());

	Mesh mesh;
	if (!loadMesh(mesh, input_path.c_str()))
		return;

	printf("Num verts (before stripping): %lu, num tris: %lu\n", mesh.vertices.size(), mesh.indices.size() / 3);

	// Mesh copy = mesh;
	// meshopt_optimizeVertexCache(&copy.indices[0], &copy.indices[0], copy.indices.size(), copy.vertices.size());
	// meshopt_optimizeVertexFetch(&copy.vertices[0], &copy.indices[0], copy.indices.size(), &copy.vertices[0], copy.vertices.size(), sizeof(Vertex));

	// Mesh copystrip = mesh;
	// meshopt_optimizeVertexCacheStrip(&copystrip.indices[0], &copystrip.indices[0], copystrip.indices.size(), copystrip.vertices.size());
	// meshopt_optimizeVertexFetch(&copystrip.vertices[0], &copystrip.indices[0], copystrip.indices.size(), &copystrip.vertices[0], copystrip.vertices.size(), sizeof(Vertex));

	// Stripify it
	std::vector<unsigned int> strip = stripify(mesh, false, 'S');

	int strips = 1;

	std::vector<Vector> positions;
	std::vector<Vector> normals;
	std::vector<Vector2> texture_coords;

	positions.reserve(strip.size());
	normals.reserve(strip.size());
	texture_coords.reserve(strip.size());

	std::reverse(strip.begin(), strip.end());

	constexpr unsigned int restart_index = ~0u;
	for (unsigned int index : strip)
	{
		//printf("Index: %u\n", index);
		if (index == restart_index)
		{
			strips++;
		}
		else
		{
			const Vertex& vertex = mesh.vertices[index];

			positions.emplace_back(vertex.px, vertex.py, vertex.pz);
			normals.emplace_back(vertex.nx, vertex.ny, vertex.nz);
			texture_coords.emplace_back(vertex.tx, vertex.ty);
		}
	}

	printf("strips: %d\n", strips);

	printf("Num verts (after stripping): %lu, num tris: %lu\n", positions.size(), positions.size() - 2);

	std::vector<std::byte> mesh_data = serialize_mesh(positions, normals, texture_coords);

	if (write_output)
	{
		printf("Writing out file: %s\n", output_path.c_str());

		std::ofstream fout;
		fout.open(output_path, std::ios::binary | std::ios::out);
		fout.write((const char*)mesh_data.data(), mesh_data.size());
		fout.close();
	}
}

static void parse_args(int argc, char** argv)
{
	cxxopts::options options("egg-ps2-mesh-converter", "Converts .obj meshes into a mesh format that the ps2 can "
	                                                   "understand by turning the mesh into strips.");

	options.add_options()("i,input", "path to the input obj mesh", cxxopts::value<std::string>());
	options.add_options()("o,output", "output path for the mesh", cxxopts::value<std::string>());
	options.add_options()("null", "null output, don't write file", cxxopts::value<bool>()->default_value("false"));
	options.add_options()("h,help", "print usage", cxxopts::value<bool>());

	options.parse_positional({"input", "output"});

	options.positional_help("[input file] [output file]");

	options.footer("If no output file is specified then the program will output the file to \"input path\" + .ps2_mesh");

	auto result = options.parse(argc, argv);

	if (result.arguments().size() == 0 || result.count("help"))
	{
		printf("%s\n", options.help().c_str());
		if (result.arguments().size() == 0)
		{
			exit(-1);
		}
		else
		{
			exit(0);
		}
	}

	input_path = result["input"].as<std::string>();

	if (result["null"].as<bool>())
	{
		write_output = false;
	}

	if (result["output"].count())
	{
		output_path = result["output"].as<std::string>();
	}
	else
	{
		output_path = input_path + ".ps2_mesh";
	}
}

int main(int argc, char** argv)
{
	parse_args(argc, argv);

	meshopt_encodeVertexVersion(0);
	meshopt_encodeIndexVersion(1);

	process();

	return 0;
}