#pragma once
#include "tamtypes.h"
#include "engine.hpp"

class tickable
{
public:
	tickable()
	{
		engine::add_tickable(this);
	};
	virtual void tick(float deltaTime) {};
};