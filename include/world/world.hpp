#pragma once

#include "egg/asset.hpp"

namespace World
{
// Creates the world
void init();

class Level;

using LevelConstructorPtr = Level* (*)(Asset::Reference);
class WorldInterface
{
public:
	virtual void initialize()                                                                             = 0;
	virtual bool load_level(Asset::Reference level)                                                       = 0;
	virtual Level* get_level(Asset::Reference level)                                                      = 0;
	virtual void register_level_constructor(Asset::Reference level, LevelConstructorPtr constructor_func) = 0;
	virtual ~WorldInterface() {};
};

WorldInterface* get();
} // namespace World