#pragma once

#include "types.hpp"

bool load_mesh(std::string_view path);

bool parseObj(std::string_view path, std::vector<Mesh>& out_meshes);

bool parseFbx(std::string_view path, std::vector<Mesh>& meshes);