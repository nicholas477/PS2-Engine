#include "mesh/model_importer.hpp"
#include "mesh/model_modifiers.hpp"

#include <json/json.h>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include "utils.hpp"
#include "app.hpp"

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

bool parseJson(std::string_view path)
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
		std::vector<Mesh> meshes;
		if (!parseMesh(path, obj, meshes))
		{
			fprintf(stderr, "Failed to parse mesh %s\n", path.data());
			return false;
		}

		printf("Loaded %lu meshes from file\n", meshes.size());

		for (size_t i = 0; i < meshes.size(); ++i)
		{
			for (const std::string& modifier : meshes[i].modifiers)
			{
				apply_modification(modifier, meshes[i]);
			}
		}

		std::vector<MeshStrip> strips;
		if (!stripify(meshes, strips))
		{
			return false;
		}

		printf(ANSI_COLOR_MAGENTA "[PS2-Mesh-Converter]: Serializing mesh to file\n" ANSI_COLOR_RESET);
		std::vector<std::byte> mesh_data = serialize_meshes(meshes[0].primitive_type, strips); // 0x0004 = GL_TRIANGLE_STRIPS

		if (write_output())
		{
			printf("Writing out file: %s\n", output_path().c_str());

			std::ofstream fout;
			fout.open(output_path(), std::ios::binary | std::ios::out);
			fout.write((const char*)mesh_data.data(), mesh_data.size());
			fout.close();
			return true;
		}
	}
	else if (iequals(type, "texture"))
	{
		printf("opening texture!\n");

		return true;
	}
	else
	{
		printf("Couldn't parse asset of type: %s\n", type.c_str());
	}

	return false;
}