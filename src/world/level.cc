#include "world/level.hpp"

#include "egg/assert.hpp"

namespace World
{
Level::Level(Asset::Reference level_reference)
{
	debug_name = "Level";
	checkf(Filesystem::load_file(level_reference, level_data, level_data_size), "unable to load level!");
	level = (LevelFileHeader*)level_data.get();

	collect_references(level_references, level->meshes);

	for (Asset::Reference ref : level_references)
	{
		meshes[ref] = Mesh(ref);

		//checkf(meshes[ref].is_valid(), "Mesh invalid!");

		//printf("\n");
	}
}

void Level::render(const GS::GSState& gs_state)
{
	for (int i = 0; i < level->meshes.mesh_files.num_elements(); ++i)
	{
		meshes[level->meshes.mesh_files[i]].draw(level->meshes.mesh_transforms[i] * gs_state.world_view);
	}
}

} // namespace World