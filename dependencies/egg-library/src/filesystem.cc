#include "egg/filesystem.hpp"

#include <cstdio>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <memory>
#include <filesystem>

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

static void convert_to_83_path(std::string_view path, char* buffer, size_t len)
{
	// Iterate backwards and look for the beginning of the filename
	size_t filename_begin  = 0;
	size_t extension_begin = path.size();
	for (int i = path.size(); i >= 0; --i)
	{
		if (path[i] == '.')
		{
			extension_begin = i;
		}
		else if (path[i] == '/' || path[i] == '\\')
		{
			filename_begin = i + 1;
			break;
		}
	}

	// Copy up to the first 8 filename characters over
	size_t itr_end = std::min(filename_begin + 8, extension_begin);
	itr_end        = std::min(itr_end, path.size());
	for (size_t i = 0; i < itr_end; ++i)
	{
		buffer[i] = path[i];
	}

	// Copy the extension over
	size_t itr_extension_end = std::min(4U, path.size() - extension_begin);
	for (size_t i = 0; i < itr_extension_end; ++i)
	{
		buffer[itr_end + i] = path[extension_begin + i];
	}
	buffer[itr_end + itr_extension_end] = '\0';
}

template <Type type>
static std::string convert_filepath_to_systempath_impl(std::string_view path)
{
	size_t host_string_length = constexpr_strlen(get_filesystem_prefix(type)) + 1; // + 1 for the filesystem separator
	host_string_length += path.length();

	check(host_string_length <= 255);

	static char out_path_chars[256];

	constexpr const char* filesystem_prefix = get_filesystem_prefix(type);
	constexpr int filesystem_prefix_len     = constexpr_strlen(filesystem_prefix);

	// Copy over the filesystem prefix to the buffer
	for (int i = 0; i < filesystem_prefix_len; ++i)
	{
		out_path_chars[i] = filesystem_prefix[i];
	}

	int index = filesystem_prefix_len;

	int path_start = 0;
	// iterate past the start of the path
	while (path[path_start] == '\\' || path[path_start] == '/')
	{
		path_start++;
	}

	path = std::string_view(path.data() + path_start, path.end());

	if constexpr (type == Type::cdrom)
	{
		convert_to_83_path(path, out_path_chars + filesystem_prefix_len, (256 - filesystem_prefix_len) - 1);
	}
	else
	{
		strncpy(out_path_chars + filesystem_prefix_len, path.data(), (256 - filesystem_prefix_len) - 1);
	}

	// Convert to uppercase, etc
	for (size_t i = filesystem_prefix_len - 1; i < host_string_length; ++i)
	{
		if (out_path_chars[i] == '\\' || out_path_chars[i] == '/')
		{
			out_path_chars[i] = get_filesystem_separator(type);
		}
		else if (out_path_chars[i] == '-')
		{
			out_path_chars[i] = '_';
		}
		else if (get_filesystem_type() == Type::cdrom)
		{
			out_path_chars[i] = toupper(out_path_chars[i]);
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

bool file_exists(const Path& path)
{
	return std::filesystem::exists(path.c_str());
}

void iterate_dir(const Path& dir, std::function<void(const Path&)> itr_func, bool recursive)
{
	checkf(itr_func, "iterate_dir called with empty itr_func!");

	namespace fs = std::filesystem;
	if (recursive)
	{
		for (const fs::directory_entry& dir_entry :
		     fs::recursive_directory_iterator(dir.c_str()))
		{
			itr_func(Path(dir_entry.path().c_str(), false));
		}
	}
	else
	{
		for (const fs::directory_entry& dir_entry :
		     fs::directory_iterator(dir.c_str()))
		{
			itr_func(Path(dir_entry.path().c_str(), false));
		}
	}
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