
#include "egg/asset.hpp"
#include "egg/filesystem.hpp"

#include <filesystem>
#include <fstream>

#include "cxxopts.hpp"

namespace fs = std::filesystem;

static std::string input_path;
static std::string output_path;
static std::string path_prefix  = "";
static bool write_output        = true;
static bool convert_path_to_iso = true;

static void parse_args(int argc, char** argv)
{
	cxxopts::options options("egg-ps2-manifest-generator", "Generates a manifest file that maps asset hashes to a path "
	                                                       "on disk.");

	options.add_options()("i,input", "base directory for the asset manifest. This should be the folder that has MAIN.00 and the assets directory", cxxopts::value<std::string>());
	options.add_options()("o,output", "output file path for the manifest", cxxopts::value<std::string>());
	options.add_options()("p,prefix", "file path prefix added to the manifest paths", cxxopts::value<std::string>());
	options.add_options()("convert_path", "convert the path to ISO filepaths. True by default", cxxopts::value<bool>()->default_value("true"));
	options.add_options()("null", "null output. Don't write file, instead print out the hash table", cxxopts::value<bool>()->default_value("false"));
	options.add_options()("h,help", "print usage", cxxopts::value<bool>());

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

	convert_path_to_iso = result["convert_path"].as<bool>();
	write_output        = result["null"].as<bool>() == false;

	if (write_output)
	{
		if (result["output"].count())
		{
			output_path = result["output"].as<std::string>();
			fs::path output_path_path(output_path);

			if (!output_path_path.has_filename())
			{
				printf("Output file path \"%s\" does not have a filename! Exiting\n", output_path_path.c_str());
				exit(-1);
			}

			if (!fs::exists(output_path_path.parent_path()))
			{
				printf("Output directory \"%s\" does not exist! Exiting\n", output_path_path.parent_path().c_str());
				exit(-1);
			}
		}
		else
		{
			printf("Output path not specified! Exiting\n");
			exit(-1);
		}
	}

	if (result["input"].count())
	{
		input_path = result["input"].as<std::string>();
		fs::path input_path_path(input_path);

		if (!fs::is_directory(input_path_path))
		{
			printf("Input directory \"%s\" is not a directory! Exiting\n", input_path_path.c_str());
			exit(-1);
		}

		if (!fs::exists(input_path))
		{
			printf("Input directory \"%s\" does not exist! Exiting\n", input_path.c_str());
			exit(-1);
		}
	}
	else
	{
		printf("Input directory not specified! Exiting\n");
		exit(-1);
	}

	if (result["prefix"].count() > 0)
	{
		path_prefix = result["prefix"].as<std::string>();
	}
}

static constexpr int max_strlen = 64;

static void print_asset_table(bool print_empty = false)
{
	for (int i = 0; i < max_strlen; ++i)
	{
		putchar('-');
	}
	printf("--------------\n");

	for (uint32_t i = 0; i < Asset::asset_hashmap_size; ++i)
	{
		if (Asset::get_asset_table().keys[i] != 0 || print_empty)
		{
			printf("|%11u|%-64s|\n", Asset::get_asset_table().keys[i], Asset::get_asset_table().values[i].data());
			for (int i = 0; i < max_strlen; ++i)
			{
				putchar('-');
			}
			printf("--------------\n");
		}
	}
}

static void print_num_entries()
{
	uint32_t num_entries = 0;

	for (uint32_t i = 0; i < Asset::asset_hashmap_size; ++i)
	{
		if (Asset::get_asset_table().keys[i] != 0)
		{
			num_entries++;
		}
	}

	printf("Num entries: %u/%lu\n", num_entries, Asset::asset_hashmap_size);
}

static void add_path_to_manifest(const fs::path& path, bool convert_to_iso)
{
	fs::path input_path_path(input_path);
	fs::path relative_path = std::filesystem::relative(path, input_path_path);

	if (path_prefix.size() > 0)
	{
		relative_path = fs::path(path_prefix + relative_path.c_str());
	}

	Filesystem::Path iso_path = Filesystem::convert_to_iso_path(relative_path.c_str());

	uint32_t hash = Asset::Reference(iso_path).hash;

	if (convert_to_iso)
	{
		Asset::add_path(Asset::Reference(hash), iso_path);
	}
	else
	{
		Asset::add_path(Asset::Reference(hash), relative_path.c_str(), false);
	}
}

static void generate_manifest()
{
	fs::path input_path_path(input_path);
	for (const std::filesystem::directory_entry& dir_entry :
	     fs::recursive_directory_iterator(input_path_path))
	{
		if (!dir_entry.is_directory())
		{
			add_path_to_manifest(dir_entry.path(), convert_path_to_iso);
		}
	}

	if (!write_output)
	{
		print_asset_table(false);
	}

	print_num_entries();
}

static void write_out_file()
{
	std::vector<std::byte> hash_table_bytes;
	Asset::serialize_asset_table(hash_table_bytes);

	std::ofstream fout;
	fout.open(output_path, std::ios::binary | std::ios::out);
	fout.write((const char*)hash_table_bytes.data(), hash_table_bytes.size());
	fout.close();
}

int main(int argc, char** argv)
{
	Filesystem::set_filesystem_type(Filesystem::Type::cdrom);
	parse_args(argc, argv);
	generate_manifest();

	if (write_output)
	{
		write_out_file();
	}
	return 0;
}