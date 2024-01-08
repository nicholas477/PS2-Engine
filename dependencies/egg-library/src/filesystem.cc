#include "egg/filesystem.hpp"

#include <cstdio>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <memory>

#include "egg/string.hpp"
#include "egg/assert.hpp"

#ifdef WIN32
#include <io.h>
#define F_OK 0
#define access _access
#else
#include <unistd.h>
#endif

namespace Filesystem
{
// 9 is the max filesystem prefix length
static std::array<char, max_path_length + 9> path_buffer {'\0'};
static Type _filesystem_type = Type::uninitialized;

Type get_filesystem_type()
{
	return _filesystem_type;
}

void set_filesystem_type(Type new_type)
{
	_filesystem_type = new_type;

	// Copy the filesystem prefix to the path buffer
	const char* filesystem_prefix   = get_filesystem_prefix(new_type);
	size_t filesystem_prefix_length = strlen(filesystem_prefix);
	strncpy(path_buffer.data(), filesystem_prefix, filesystem_prefix_length);
}

bool load_file(const Path& path, std::vector<std::byte>& out_bytes)
{
	FILE* file = fopen(path.to_full_filepath(), "r");

	if (file != nullptr)
	{
		// Read the file size
		fseek(file, 0, SEEK_END);
		const size_t file_size = ftell(file);
		out_bytes.resize(file_size, std::byte(0));

		// Seek back to the beginning
		fseek(file, 0, SEEK_SET);

		fread((char*)out_bytes.data(), 1, file_size, file);

		fclose(file);
		return true;
	}

	return false;
}

bool load_file(const Path& path, std::unique_ptr<std::byte[]>& out_bytes, size_t& size, size_t alignment)
{
	FILE* file = fopen(path.to_full_filepath(), "r");

	if (file != nullptr)
	{
		// Read the file size
		fseek(file, 0, SEEK_END);
		size = ftell(file);
#ifdef _MSC_VER
		out_bytes = std::unique_ptr<std::byte[]>((std::byte*)operator new[](size, (std::align_val_t)alignment));
#else
		out_bytes = std::unique_ptr<std::byte[]>(new (std::align_val_t(alignment)) std::byte[size]);
#endif

		// Seek back to the beginning
		fseek(file, 0, SEEK_SET);

		fread((char*)out_bytes.get(), 1, size, file);
		fclose(file);
		return true;
	}

	return false;
}

bool file_exists(const Path& path)
{
	return access(path.to_full_filepath(), F_OK) == 0;
}

// void iterate_dir(const Path& dir, std::function<void(const Path&)> itr_func, bool recursive)
// {
// 	checkf(itr_func, "iterate_dir called with empty itr_func!");

// 	namespace fs = std::filesystem;
// 	if (recursive)
// 	{
// 		for (const fs::directory_entry& dir_entry :
// 		     fs::recursive_directory_iterator(dir.to_full_filepath()))
// 		{
// 			itr_func(Path(dir_entry.path().c_str(), false));
// 		}
// 	}
// 	else
// 	{
// 		for (const fs::directory_entry& dir_entry :
// 		     fs::directory_iterator(dir.to_full_filepath()))
// 		{
// 			itr_func(Path(dir_entry.path().c_str(), false));
// 		}
// 	}
// }

const char* Path::to_full_filepath(Type in_filesystem_type) const
{
	size_t filesystem_prefix_length = strlen(get_filesystem_prefix(in_filesystem_type));
	strncpy(path_buffer.data() + filesystem_prefix_length, mem.data(), max_path_length);

	return path_buffer.data();
}

// void run_tests()
// {
// 	switch (get_filesystem_type())
// 	{
// 		case Type::cdrom: {
// 			const std::string converted_path           = convert_filepath_to_systempath_impl<Type::cdrom>("audsrv.irx");
// 			constexpr std::string_view expected_string = "cdrom0:\\AUDSRV.IRX";
// 			if (converted_path != expected_string)
// 			{
// 				printf("Filesystem check failed!\n");
// 				printf("Input string: %s\n", "audsrv.irx");
// 				printf("Converted string: %s\n", converted_path.c_str());
// 				printf("Expected string: %s\n", expected_string.data());
// 				check(false);
// 			}
// 			break;
// 		}

// 		default:
// 			break;
// 	}
// }
} // namespace Filesystem