
#include <stdio.h>
#include <assert.h>
#include <fstream>
#include <iterator>
#include <egg/math_types.hpp>
#include <cstddef>
#include <algorithm>
#include "types.hpp"

#include "model_importer.hpp"
#include "model_exporter.hpp"
#include <cxxopts.hpp>

#include "meshoptimizer.h"

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

static std::string input_path;
static std::string output_path;
static bool write_output = true;

static void process()
{
	printf("Processing model file: %s\n", input_path.c_str());

	Mesh mesh;
	if (!load_mesh(mesh, input_path.c_str()))
	{
		fprintf(stderr, "Failed to load model\n");
		std::exit(-1);
		return;
	}

	printf("Num verts (before stripping): %lu, num tris: %lu\n", mesh.vertices.size(), mesh.indices.size() / 3);

	// Stripify it
	std::vector<unsigned int> strip = stripify(mesh, false, 'S');

	int strips = 1;

	std::vector<Vector> positions;
	std::vector<Vector> normals;
	std::vector<Vector2> texture_coords;
	std::vector<Vector> colors;

	positions.reserve(strip.size());
	normals.reserve(strip.size());
	texture_coords.reserve(strip.size());
	colors.reserve(strip.size());

	// Reverse winding order
	std::reverse(strip.begin(), strip.end());

	constexpr unsigned int restart_index = ~0u;
	for (unsigned int index : strip)
	{
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
			colors.emplace_back(vertex.r, vertex.g, vertex.b);
		}
	}

	printf("strips: %d\n", strips);

	printf("Num verts (after stripping): %lu, num tris: %lu\n", positions.size(), positions.size() - 2);

	std::vector<std::byte> mesh_data = serialize_mesh(positions, normals, texture_coords, colors);

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