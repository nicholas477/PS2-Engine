#pragma once

#include <cstddef>
#include <vector>
#include <string>

namespace filesystem
{
enum class filesystem_type {
	cdrom,
	host
};

filesystem_type get_filesystem_type();
void set_filesystem_type(filesystem_type new_type);

static constexpr const char* get_filesystem_prefix(filesystem_type in_filesystem_type = get_filesystem_type())
{
	switch (in_filesystem_type)
	{
		case filesystem_type::cdrom:
			return "cdrom0:";
		case filesystem_type::host:
			return "host0:";
	}

	return 0;
}

static constexpr char get_filesystem_separator(filesystem_type in_filesystem_type = get_filesystem_type())
{
	switch (in_filesystem_type)
	{
		case filesystem_type::cdrom:
			return '\\';
		case filesystem_type::host:
			return '/';
	}

	return '\0';
}

std::string convert_filepath_to_systempath(const std::string& path);

// Returns true if the file was loaded succesfully
bool load_file(const std::string& path, std::vector<std::byte>& out_bytes);

} // namespace filesystem