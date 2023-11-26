#pragma once

#include "types.hpp"

bool load_mesh(std::vector<Mesh>& out_meshes, std::string_view path);

bool parseFbx(std::string_view path, std::vector<Mesh>& meshes);