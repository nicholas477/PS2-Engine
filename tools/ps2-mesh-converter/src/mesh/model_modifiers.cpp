#include "mesh/model_modifiers.hpp"
#include "utils.hpp"
#include "meshoptimizer.h"
#include <cmath>
#include <algorithm>

static void apply_gamma_correction(Mesh& mesh)
{
	for (Vertex& vertex : mesh.vertices)
	{
		//vertex.r = pow(vertex.r, 1.0 / 2.2);
		//vertex.g = pow(vertex.g, 1.0 / 2.2);
		//vertex.b = pow(vertex.b, 1.0 / 2.2);
		vertex.a = pow(vertex.a, 1.0 / 2.2);
	}
}

// Converts the red channel to an integer
static void apply_vegetation_mod(Mesh& mesh)
{
	for (Vertex& vertex : mesh.vertices)
	{
		int32_t red_value = (int)std::round(vertex.r * 255.f);
		vertex.r          = *(reinterpret_cast<float*>(&red_value));
	}
}

bool apply_modification(const std::string& mod, Mesh& mesh)
{
	if (mod == "vegetation")
	{
		printf(ANSI_COLOR_MAGENTA "[PS2-Mesh-Converter]: Applying mesh modification: %s" ANSI_COLOR_RESET "\n", "vegetation");
		apply_vegetation_mod(mesh);
		return true;
	}
	else if (mod == "gamma_correct_alpha")
	{
		printf(ANSI_COLOR_MAGENTA "[PS2-Mesh-Converter]: Applying mesh modification: %s" ANSI_COLOR_RESET "\n", "gamma correction");
		apply_gamma_correction(mesh);
		return true;
	}

	printf(ANSI_COLOR_RED "[PS2-Mesh-Converter]: Unable to find mesh mod: %s" ANSI_COLOR_RESET "\n", mod.c_str());
	return false;
}

static std::vector<unsigned int> stripify(const Mesh& mesh, bool use_restart, char desc)
{
	unsigned int restart_index = use_restart ? ~0u : 0;

	// note: input mesh is assumed to be optimized for vertex cache and vertex fetch
	std::vector<unsigned int> strip(meshopt_stripifyBound(mesh.indices.size()));
	strip.resize(meshopt_stripify(&strip[0], &mesh.indices[0], mesh.indices.size(), mesh.vertices.size(), restart_index));

	return strip;
}

bool stripify(const std::vector<Mesh>& meshes, std::vector<MeshStrip>& out_strips)
{
	meshopt_encodeVertexVersion(0);
	meshopt_encodeIndexVersion(1);

	for (size_t i = 0; i < meshes.size(); ++i)
	{
		// Stripify it
		std::vector<unsigned int> strip = stripify(meshes[i], false, 'S');

		int num_strips = 1;

		MeshStrip& new_strip                 = out_strips.emplace_back();
		std::vector<Vector>& positions       = new_strip.positions;
		std::vector<Vector>& normals         = new_strip.normals;
		std::vector<Vector2>& texture_coords = new_strip.texture_coords;
		std::vector<Vector>& colors          = new_strip.colors;

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
				num_strips++;
			}
			else
			{
				const Vertex& vertex = meshes[i].vertices[index];

				positions.emplace_back(vertex.px, vertex.py, vertex.pz);
				normals.emplace_back(vertex.nx, vertex.ny, vertex.nz);
				texture_coords.emplace_back(vertex.tx, vertex.ty);
				colors.emplace_back(vertex.r, vertex.g, vertex.b, vertex.a);
			}
		}
	}

	return true;
}