#pragma once

#include <cstddef>
#include <vector>
#include <string>
#include <string_view>
#include <memory>

namespace Filesystem
{
enum class Type {
	cdrom,
	host,
	rom
};

Type get_filesystem_type();
void set_filesystem_type(Type new_type);

static constexpr const char* get_filesystem_prefix(Type in_filesystem_type = get_filesystem_type())
{
	switch (in_filesystem_type)
	{
		case Type::cdrom:
			return "cdrom0:";
		case Type::host:
			return "host0:";
		case Type::rom:
			return "rom0:";
	}

	return 0;
}

static constexpr char get_filesystem_separator(Type in_filesystem_type = get_filesystem_type())
{
	switch (in_filesystem_type)
	{
		case Type::cdrom:
			return '\\';
		case Type::host:
		case Type::rom:
			return '/';
	}

	return '\0';
}

std::string convert_filepath_to_systempath(std::string_view path, Type in_filesystem_type = get_filesystem_type());

struct Path
{
	Path(const std::string& in_path, bool convert_path = true)
	{
		if (convert_path)
		{
			_path_str = convert_filepath_to_systempath(in_path);
		}
		else
		{
			_path_str = in_path;
		}
	}

	inline const char* c_str() const
	{
		return _path_str.c_str();
	}

	operator const std::string &() const
	{
		return _path_str;
	}

	operator std::string_view() const
	{
		return _path_str;
	}

	std::string _path_str;
};

// Returns true if the file was loaded succesfully
bool load_file(const Path& path, std::vector<std::byte>& out_bytes);
bool load_file(const Path& path, std::unique_ptr<std::byte[]>& out_bytes, size_t alignment = 1);

} // namespace Filesystem

// Path literal constructor
//
// Converts the path to a system path. I.e: assets/sounds/vine_boom.adpcm -> cdrom0:/ASSETS/SOUNDS/VINE_BOOM.ADPCM
static Filesystem::Path operator""_p(const char* p, std::size_t)
{
	return Filesystem::Path(p);
}