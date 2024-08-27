#pragma once
#include "egg/math_types.hpp"

#include <vector>

namespace Json
{
class Value;
}

bool parseTexture(std::string_view path, const Json::Value& obj, std::vector<std::byte>& out_data);