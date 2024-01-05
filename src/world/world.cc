// #include "input/gamepad.hpp"
#include "world/world.hpp"
#include "world/level.hpp"

#include "egg/asset.hpp"

#include <memory>

namespace World
{
class World final: public WorldInterface
{
public:
	virtual void initialize() override
	{
		printf("Initializing levels\n");
		for (auto& [_, level] : levels)
		{
			level->initialize();
		}
	}

	virtual bool load_level(Asset::Reference level) override
	{
		check(levels.find(level) == levels.end());
		printf("Loading level: %s\n", Asset::lookup_path(level).data());

		levels[level] = level_constructors[level](level);

		printf("Level loaded.\n");
		return true;
	}

	virtual Level* get_level(Asset::Reference level) override
	{
		auto itr = levels.find(level);

		if (itr != levels.end())
		{
			return itr->second;
		}

		return nullptr;
	}

	virtual void register_level_constructor(Asset::Reference level, LevelConstructorPtr constructor_func)
	{
		check(level_constructors.find(level) == level_constructors.end());

		level_constructors[level] = constructor_func;
	}

	std::unordered_map<Asset::Reference, Level*> levels;
	std::unordered_map<Asset::Reference, LevelConstructorPtr> level_constructors;
};

void init()
{
	printf("World::Init()\n");

	World* world = (World*)get();
	world->load_level("assets/levels/new_level.lvl"_asset);

	printf("Initializing world...\n");
	world->initialize();

	printf("World initialized!\n");
}

WorldInterface* get()
{
	static World world;
	return &world;
}
} // namespace World