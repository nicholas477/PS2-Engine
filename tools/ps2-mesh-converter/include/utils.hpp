#pragma once
#include <cstdint>
#include <string>
#include <string.h>

enum class printf_color : uint8_t {
	none    = 0,
	red     = 31,
	green   = 32,
	yellow  = 33,
	blue    = 34,
	magenta = 35,
	cyan    = 36,
};

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

// template <printf_color color = printf_color::none>
// int mesh_converter_printf(const char* str, ...)
// {
// 	std::string print_string = "\x1b[" + std::to_string(static_cast<int>(color)) + "m";
// 	print_string += str;
// 	print_string += "\x1b[0m";
// 	return printf(print_string.c_str(), ...);
// }