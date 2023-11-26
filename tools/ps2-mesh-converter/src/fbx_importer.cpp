#include "model_importer.hpp"
#include "types.hpp"

#include <filesystem>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>       // Output data structure
#include <assimp/postprocess.h> // Post processing flags

static Mesh fbxProcessMesh(aiMesh* mesh, const aiScene* scene)
{
	Mesh out;

	printf("Processing fbx mesh \"%s\"\n", mesh->mName.C_Str());
	if (mesh->HasVertexColors(0))
	{
		printf("Mesh \"%s\" has colors!\n", mesh->mName.C_Str());
	}

	out.vertices.reserve(mesh->mNumVertices);

	for (size_t i = 0; i < mesh->mNumVertices; ++i)
	{
		Vertex& v = out.vertices.emplace_back();
		v.px      = mesh->mVertices[i].x;
		v.py      = mesh->mVertices[i].y;
		v.pz      = mesh->mVertices[i].z;

		std::swap(v.py, v.pz);

		assert(mesh->HasNormals());
		if (mesh->HasNormals())
		{
			v.nx = mesh->mNormals[i].x;
			v.ny = mesh->mNormals[i].y;
			v.nz = mesh->mNormals[i].z;
			std::swap(v.ny, v.nz);
		}

		if (mesh->HasTextureCoords(0))
		{
			v.tx = mesh->mTextureCoords[0][i].x;
			v.ty = mesh->mTextureCoords[0][i].y;
		}
		else
		{
			v.tx = 0.f;
			v.ty = 0.f;
		}

		if (mesh->HasVertexColors(0))
		{
			v.r = mesh->mColors[0][i].r;
			v.g = mesh->mColors[0][i].g;
			v.b = mesh->mColors[0][i].b;
			v.a = mesh->mColors[0][i].a;
		}
		else
		{
			v.r = 1.f;
			v.g = 1.f;
			v.b = 1.f;
			v.a = 1.f;
		}
	}

	out.indices.reserve(mesh->mNumFaces * 3);
	for (size_t i = 0; i < mesh->mNumFaces; ++i)
	{
		assert(mesh->mFaces[i].mNumIndices == 3);
		out.indices.push_back(mesh->mFaces[i].mIndices[0]);
		out.indices.push_back(mesh->mFaces[i].mIndices[1]);
		out.indices.push_back(mesh->mFaces[i].mIndices[2]);
	}

	return out;
}

static void fbxProcessNode(aiNode* node, const aiScene* scene, std::vector<Mesh>& meshes)
{
	// process all the node's meshes (if any)
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(fbxProcessMesh(mesh, scene));
	}

	// then do the same for each of its children
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		fbxProcessNode(node->mChildren[i], scene, meshes);
	}
}

bool parseFbx(std::string_view path, std::vector<Mesh>& meshes)
{
	Assimp::Importer importer;

	// And have it read the given file with some example postprocessing
	// Usually - if speed is not the most important aspect for you - you'll
	// probably to request more postprocessing than we do in this example.
	const aiScene* scene = importer.ReadFile(path.data(),
	                                         aiProcess_CalcTangentSpace |
	                                             aiProcess_Triangulate |
	                                             aiProcess_JoinIdenticalVertices |
	                                             aiProcess_SortByPType |
	                                             aiProcess_PreTransformVertices);

	// If the import failed, report it
	if (scene == nullptr)
	{
		fprintf(stderr, "%s\n", importer.GetErrorString());
		return false;
	}

	fbxProcessNode(scene->mRootNode, scene, meshes);

	return true;
}