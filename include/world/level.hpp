#pragma once

#include "egg/level.hpp"
#include "egg/asset.hpp"
#include "renderer/mesh.hpp"
#include "renderer/renderable.hpp"

#include <memory>
#include <unordered_map>
#include <unordered_set>

namespace World
{
class Level: public Renderable
{
public:
	Level(Asset::Reference level_reference);

	virtual void initialize() {};
	virtual void render(const GS::GSState& gs_state) override;

	virtual ~Level() {};


public: // Data members
	size_t level_data_size;
	std::unique_ptr<std::byte[]> level_data;
	LevelFileHeader* level;

	std::unordered_set<Asset::Reference> level_references;
	std::unordered_map<Asset::Reference, Mesh> meshes;
};
} // namespace World