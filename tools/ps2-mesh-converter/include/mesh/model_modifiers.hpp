#pragma once

#include <vector>
#include "types.hpp"
#include "model_exporter.hpp"

bool apply_modification(const std::string& mod, Mesh& mesh);

bool stripify(const std::vector<Mesh>& meshes, std::vector<MeshStrip>& out_strips);