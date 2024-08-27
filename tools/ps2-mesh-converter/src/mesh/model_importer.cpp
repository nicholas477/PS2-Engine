#include "mesh/model_importer.hpp"
#include "app.hpp"

#include <algorithm>

#include "meshoptimizer.h"

#define FAST_OBJ_IMPLEMENTATION
#include "../extern/fast_obj.h"

const size_t kCacheSize = 16;

bool parseObj(std::string_view path, std::vector<Mesh>& out_meshes)
{
	// This importer only supports a single mesh
	out_meshes = {Mesh()};

	fastObjMesh* obj = fast_obj_read(path.data());
	if (!obj)
	{
		print("Error loading \"%s\": file not found", path.data());
		return false;
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
			    };

			if (obj->colors)
			{
				v.r = obj->colors[gi.p * 3 + 0];
				v.g = obj->colors[gi.p * 3 + 1];
				v.b = obj->colors[gi.p * 3 + 2];
				v.a = 1.f;
			}
			else
			{
				v.r = 1.f;
				v.g = 1.f;
				v.b = 1.f;
				v.a = 1.f;
			}

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

	std::vector<unsigned int> remap(total_indices);

	size_t total_vertices = meshopt_generateVertexRemap(&remap[0], NULL, total_indices, &vertices[0], total_indices, sizeof(Vertex));

	out_meshes[0].primitive_type = 4; // GL_TRIANGLE_STRIPS

	out_meshes[0].indices.resize(total_indices);
	meshopt_remapIndexBuffer(&out_meshes[0].indices[0], NULL, total_indices, &remap[0]);

	out_meshes[0].vertices.resize(total_vertices);
	meshopt_remapVertexBuffer(&out_meshes[0].vertices[0], &vertices[0], total_indices, sizeof(Vertex), &remap[0]);

	return true;
}