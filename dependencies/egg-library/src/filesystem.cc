#include "egg/filesystem.hpp"

#include <cstdio>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <memory>

#include "egg/assert.hpp"

namespace Filesystem
{
static Type _filesystem_type = Type::uninitialized;
Type get_filesystem_type()
{
	return _filesystem_type;
}

void set_filesystem_type(Type new_type)
{
	_filesystem_type = new_type;
}

static constexpr std::size_t constexpr_strlen(const char* s)
{
	return std::char_traits<char>::length(s);
}

template <Type type>
static std::string convert_filepath_to_systempath_impl(std::string_view path)
{
	size_t host_string_length = constexpr_strlen(get_filesystem_prefix(type)) + 1; // + 1 for the filesystem separator
	host_string_length += path.length();

	check(host_string_length <= 255);

	static char out_path_chars[256];

	// snprintf(out_path_chars, host_string_length + 1, "%s%c%.*s",
	//          get_filesystem_prefix(type), get_filesystem_separator(type), path.length(), path.data());

	constexpr const char* filesystem_prefix = get_filesystem_prefix(type);
	constexpr int filesystem_prefix_len     = constexpr_strlen(filesystem_prefix);

	// Copy over the filesystem prefix to the buffer
	for (int i = 0; i < filesystem_prefix_len; ++i)
	{
		out_path_chars[i] = filesystem_prefix[i];
	}

	int index = filesystem_prefix_len;

	if (path[0] != '\\' || path[0] != '/')
	{
		out_path_chars[index] = get_filesystem_separator();
		index++;
	}

	// Write out the filename
	int filename_start_index  = -1;
	int extension_start_index = -1;
	for (size_t i = 0; i < host_string_length; ++i)
	{
		if (path[i] == '\\' || path[i] == '/')
		{
			out_path_chars[index + i] = get_filesystem_separator(type);

			filename_start_index = i + 1;
		}
		else if (path[i] == '-')
		{
			out_path_chars[index + i] = '_';
		}
		else if (get_filesystem_type() == Type::cdrom)
		{
			out_path_chars[index + i] = toupper(path[i]);
		}
		else
		{
			out_path_chars[index + i] = path[i];
		}

		if (path[i] == '.')
		{
			extension_start_index = i + 1;
		}
	}

	return std::string(out_path_chars);
}

std::string convert_filepath_to_systempath(std::string_view path, Type in_filesystem_type)
{
	switch (in_filesystem_type)
	{
		case Type::cdrom:
			return convert_filepath_to_systempath_impl<Type::cdrom>(path);

		case Type::host:
			return convert_filepath_to_systempath_impl<Type::host>(path);

		case Type::rom:
			return convert_filepath_to_systempath_impl<Type::rom>(path);

		case Type::uninitialized:
			checkf(false, "Tried to convert filepath to systempath on an uninitialized filesystem!\n");
	}

	check(false);
	return "";
}

bool load_file(const Path& path, std::vector<std::byte>& out_bytes)
{
	std::ifstream file(path, std::ios::binary);
	if (file.is_open() && file.good())
	{
		// Read the file size
		file.seekg(0, std::ios::end);
		const size_t file_size = file.tellg();
		out_bytes.resize(file_size, std::byte(0));

		// Seek back to the beginning
		file.seekg(0, std::ios::beg);

		file.read((char*)out_bytes.data(), file_size);
		return true;
	}

	return false;
}

bool load_file(const Path& path, std::unique_ptr<std::byte[]>& out_bytes, size_t& size, size_t alignment)
{
	std::ifstream file(path, std::ios::binary);
	if (file.is_open() && file.good())
	{
		// Read the file size
		file.seekg(0, std::ios::end);
		size      = file.tellg();
		out_bytes = std::unique_ptr<std::byte[]>(new (std::align_val_t(alignment)) std::byte[size]);

		// Seek back to the beginning
		file.seekg(0, std::ios::beg);

		file.read((char*)out_bytes.get(), size);
		return true;
	}

	return false;
}

void run_tests()
{
	switch (get_filesystem_type())
	{
		case Type::cdrom: {
			const std::string converted_path           = convert_filepath_to_systempath_impl<Type::cdrom>("audsrv.irx");
			constexpr std::string_view expected_string = "cdrom0:\\AUDSRV.IRX";
			if (converted_path != expected_string)
			{
				printf("Filesystem check failed!\n");
				printf("Input string: %s\n", "audsrv.irx");
				printf("Converted string: %s\n", converted_path.c_str());
				printf("Expected string: %s\n", expected_string.data());
				check(false);
			}
			break;
		}

		default:
			break;
	}
}
} // namespace Filesystem