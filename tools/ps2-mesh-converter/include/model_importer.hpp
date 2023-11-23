#pragma once

#include "types.hpp"
#include <assimp/Importer.hpp>

bool load_mesh(Mesh& mesh, std::string_view path);