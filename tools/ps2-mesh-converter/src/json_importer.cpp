#include "model_importer.hpp"

#include <jsoncpp/json/json.h>
#include <fstream>
#include <filesystem>

bool parseJson(std::string_view path, std::vector<Mesh>& meshes)
{
	std::ifstream ifs(path.data());
	Json::Value obj;

	{
		Json::Reader reader;
		reader.parse(ifs, obj);
	}

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

	return true;
}