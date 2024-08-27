#include "app.hpp"

#include <stdio.h>
#include <stdarg.h>
#include <vector>
#include <functional>

struct logs
{
	std::vector<std::function<void()>> print_funcs;

	~logs()
	{
		for (const auto& func : print_funcs)
		{
			func();
		}
	}
} _logs;

static std::string log_prefix()
{
	return std::string("[") + APP_NAME.data() + "]: ";
}

void print(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	const std::string combined_str = std::string(ANSI_COLOR_MAGENTA) + log_prefix() + fmt + ANSI_COLOR_RESET + "\n";

	std::array<char, 1024> buffer;
	vsnprintf(buffer.data(), buffer.size(), combined_str.c_str(), args);
	va_end(args);

	_logs.print_funcs.emplace_back([=]() {
		fprintf(stdout, "%s", buffer.data());
	});
}

void print_color_override(const char* color_override, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	const std::string combined_str = std::string(color_override) + log_prefix() + fmt + ANSI_COLOR_RESET + "\n";

	std::array<char, 1024> buffer;
	vsnprintf(buffer.data(), buffer.size(), combined_str.c_str(), args);
	va_end(args);

	_logs.print_funcs.emplace_back([=]() {
		fprintf(stdout, "%s", buffer.data());
	});
}

void print_error(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	const std::string combined_str = std::string(ANSI_COLOR_RED) + log_prefix() + fmt + ANSI_COLOR_RESET + "\n";

	std::array<char, 1024> buffer;
	vsnprintf(buffer.data(), buffer.size(), combined_str.c_str(), args);
	va_end(args);

	_logs.print_funcs.emplace_back([=]() {
		fprintf(stderr, "%s", buffer.data());
	});
}
