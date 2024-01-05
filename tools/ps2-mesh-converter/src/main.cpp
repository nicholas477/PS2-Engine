
#include <stdio.h>
#include <assert.h>
#include <fstream>
#include <filesystem>
#include <iterator>
#include <egg/math_types.hpp>
#include <cstddef>
#include <algorithm>
#include "types.hpp"
#include "utils.hpp"

#include "mesh/model_importer.hpp"
#include "mesh/model_exporter.hpp"
#include "mesh/model_modifiers.hpp"
#include "json_importer.hpp"
#include <cxxopts.hpp>

bool& write_output()
{
	static bool write_output = true;
	return write_output;
}

std::string& input_path()
{
	static std::string input_path;
	return input_path;
}

std::string& output_path()
{
	static std::string output_path;
	return output_path;
}

bool load_file(std::string_view path)
{
	std::filesystem::path p(path);

	bool parsed = false;
	if (iequals(p.extension(), ".json"))
	{
		parsed = parseJson(path);
	}

	if (parsed == false)
	{
		printf("Couldn't find/parse asset at path %s\n", path.data());
		return false;
	}

	return true;
}

int& argc()
{
	static int argc;
	return argc;
}

char**& argv()
{
	static char** argv;
	return argv;
}

static void process()
{
	printf(ANSI_COLOR_GREEN "[PS2-Mesh-Converter]: Starting" ANSI_COLOR_RESET "\n");
	printf(ANSI_COLOR_MAGENTA "[PS2-Mesh-Converter]: Loading file" ANSI_COLOR_RESET "\n");
	printf("Processing file: %s\n", input_path().c_str());

	if (!load_file(input_path().c_str()))
	{
		std::exit(-1);
		return;
	}

	printf(ANSI_COLOR_GREEN "[PS2-Mesh-Converter]: Done\n" ANSI_COLOR_RESET);
}

static void parse_args(int argc, char** argv)
{
	cxxopts::options options("egg-ps2-mesh-converter", "Converts json assets into a cooked asset format that the ps2 can "
	                                                   "understand.");

	options.add_options()("i,input", "path to the input file", cxxopts::value<std::string>());
	options.add_options()("o,output", "output path for the file", cxxopts::value<std::string>());
	options.add_options()("null", "null output, don't write file", cxxopts::value<bool>()->default_value("false"));
	options.add_options()("h,help", "print usage", cxxopts::value<bool>());

	options.parse_positional({"input", "output"});

	options.positional_help("[input file] [output file]");

	options.footer("If no output file is specified then the program will output the file to \"input path\" + .type");

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

	input_path() = result["input"].as<std::string>();

	if (result["null"].count() && result["null"].as<bool>())
	{
		write_output() = false;
	}

	if (result["output"].count())
	{
		output_path() = result["output"].as<std::string>();
	}
	else
	{
		printf("Output path not specified\n");
		exit(-1);
	}
}

int main(int argc, char** argv)
{
	::argc() = argc;
	::argv() = argv;

	parse_args(argc, argv);

	process();

	return 0;
}