#include "mesh/model_importer.hpp"
#include "mesh/model_modifiers.hpp"
#include "texture/texture.hpp"

#include <json/json.h>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include "utils.hpp"
#include "app.hpp"

static std::filesystem::path get_file_path(std::string_view json_path, const Json::Value& obj)
{
	const std::string input_file_path = obj["file"].asString();
	print("Input file path: %s", input_file_path.c_str());

	return std::filesystem::path(json_path).parent_path() / input_file_path;
}

static bool parseMesh(std::string_view json_path, const Json::Value& obj, std::vector<Mesh>& meshes)
{
	std::filesystem::path mesh_file_path = get_file_path(json_path, obj);

	if (!std::filesystem::exists(mesh_file_path))
	{
		print_error("Couldn't find mesh at path \"%s\"!", mesh_file_path.c_str());
		return false;
	}

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
		print_error("Couldn't parse mesh at path \"%s\"!", mesh_file_path.c_str());
		return false;
	}

	if (meshes.empty() || std::all_of(meshes.begin(), meshes.end(), [](const Mesh& mesh) {
		    return mesh.indices.empty();
	    }))
	{
		print_error("Mesh data empty %s", mesh_file_path.c_str());
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

	const std::string type = obj["type"].asString();
	std::vector<std::byte> out_data;

	if (iequals(type, "mesh"))
	{
		std::vector<Mesh> meshes;
		if (!parseMesh(path, obj, meshes))
		{
			print_error("Failed to parse mesh %s", path.data());
			return false;
		}

		print("Loaded %lu meshes from file", meshes.size());

		for (const Json::Value& modifier : obj["modifiers"])
		{
			print("Applying mesh modification: %s", modifier.asCString());
			for (size_t i = 0; i < meshes.size(); ++i)
			{
				apply_modification(modifier.asString(), meshes[i]);
			}
		}

		std::vector<MeshStrip> strips;
		if (!stripify(meshes, strips))
		{
			return false;
		}

		print("Serializing mesh to file");
		out_data = serialize_meshes(meshes[0].primitive_type, strips);
	}
	else if (iequals(type, "texture"))
	{
		if (!parseTexture(path, obj, out_data))
		{
			print_error("Failed to parse texture %s", path.data());
			return false;
		}
	}
	else
	{
		print_error("Couldn't parse asset of type: %s", type.c_str());
		return false;
	}

	if (write_output())
	{
		if (out_data.size() > 0)
		{
			print("Writing out file: %s", output_path().c_str());

			std::ofstream fout;
			fout.open(output_path(), std::ios::binary | std::ios::out);
			fout.write((const char*)out_data.data(), out_data.size());
			fout.close();
			return true;
		}
		else
		{
			print_error("Couldn't write asset, output data array was empty!");
			return false;
		}
	}

	return false;
}