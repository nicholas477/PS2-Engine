#include "mesh/model_modifiers.hpp"
#include "utils.hpp"

#include <cmath>

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
		printf(ANSI_COLOR_MAGENTA "[PS2-Mesh-Converter]: Applying mesh modification: %s\n" ANSI_COLOR_RESET, "vegetation");
		apply_vegetation_mod(mesh);
		return true;
	}
	else if (mod == "gamma_correct_alpha")
	{
		printf(ANSI_COLOR_MAGENTA "[PS2-Mesh-Converter]: Applying mesh modification: %s\n" ANSI_COLOR_RESET, "gamma correction");
		apply_gamma_correction(mesh);
		return true;
	}

	printf(ANSI_COLOR_RED "[PS2-Mesh-Converter]: Unable to find mesh mod: %s\n" ANSI_COLOR_RESET, mod.c_str());
	return false;
}