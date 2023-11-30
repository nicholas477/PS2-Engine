#include "model_modifiers.hpp"

void apply_gamma_correction(std::vector<Mesh>& meshes)
{
	for (Mesh& mesh : meshes)
	{
		for (Vertex& vertex : mesh.vertices)
		{
			vertex.r = pow(vertex.r, 1.0 / 2.2);
			vertex.g = pow(vertex.g, 1.0 / 2.2);
			vertex.b = pow(vertex.b, 1.0 / 2.2);
			vertex.a = pow(vertex.a, 1.0 / 2.2);
		}
	}
}