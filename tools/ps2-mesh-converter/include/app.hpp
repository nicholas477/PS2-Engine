#pragma once

#include "utils.hpp"

#include <string>
#include <string_view>

static constexpr std::string_view APP_NAME = "PS2-Asset-Converter";

bool& write_output();
std::string& input_path();
std::string& output_path();

void print(const char* fmt, ...);
void print_color_override(const char* color_override, const char* fmt, ...);
void print_error(const char* fmt, ...);

int& argc();
char**& argv();