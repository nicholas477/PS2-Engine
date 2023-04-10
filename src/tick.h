#pragma once
#include "tamtypes.h"
#include "engine.h"

class tickable
{
public:
	tickable()
	{
		engine::add_tickable(this);
	};
	virtual void tick(float deltaTime) {};
};