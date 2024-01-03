#include "mesh/model_importer.hpp"

#include <json/json.h>
#include <fstream>
#include <filesystem>
#include <algorithm>

static bool parseMesh(std::string_view path, const Json::Value& obj, std::vector<Mesh>& meshes)
{
	std::string mesh_file = obj["mesh_file"].asString();

	std::filesystem::path mesh_file_path = std::filesystem::path(path).parent_path() / mesh_file;

	bool parsed = false;
	if (iequals(mesh_file_path.extension(), ".obj"))
	{
		parsed = parseObj(mesh_file_path.string(), meshes);
	}
	else if (iequals(mesh_file_path.extension(), ".fbx"))
	{
		parsed = parseFbx(mesh_file_path.string(), meshes);
	}

	if (parsed == false)
	{
		printf("Couldn't find/parse mesh at path %s\n", mesh_file_path.string().c_str());
		return false;
	}

	if (meshes.empty() || std::all_of(meshes.begin(), meshes.end(), [](const Mesh& mesh) {
		    return mesh.indices.empty();
	    }))
	{
		fprintf(stderr, "Mesh data empty %s\n", path.data());
		return false;
	}


	meshes[0].primitive_type = 4;
	if (obj.isMember("primitive_type") && obj["primitive_type"].isUInt())
	{
		meshes[0].primitive_type = obj["primitive_type"].asUInt();
	}

	meshes[0].modifiers.clear();
	if (obj.isMember("modifiers"))
	{
		for (auto val : obj["modifiers"])
		{
			meshes[0].modifiers.push_back(val.asString());
		}
	}

	return true;
}

bool parseJson(std::string_view path, std::vector<Mesh>& meshes)
{
	std::ifstream ifs(path.data());
	Json::Value obj;

	{
		Json::Reader reader;
		reader.parse(ifs, obj);
	}

	std::string type = obj["type"].asString();

	if (iequals(type, "mesh"))
	{
		return parseMesh(path, obj, meshes);
	}
	else
	{
		printf("Couldn't parse asset of type: %s\n", type.c_str());
	}

	return false;
}