#pragma once

#include <cstddef>
#include <string>
#include <string_view>

static constexpr size_t constexpr_strlen(const char* s)
{
	return std::char_traits<char>::length(s);
}

static_assert(constexpr_strlen("asdf") == 4);

static constexpr char charToLower(const char c)
{
	return (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c;
}

static constexpr char charToUpper(const char c)
{
	return (c >= 'a' && c <= 'z') ? c - ('a' - 'A') : c;
}

static_assert(charToUpper('a') == 'A');
static_assert(charToLower('A') == 'a');