#pragma once
#include "utils/list.hpp"

// This uses an intrusive linked list since objects can be created/deleted during tick
// If we used an array of pointers it would be invalidated
class Tickable: public TIntrusiveLinkedList<Tickable>
{
public:
	virtual void tick(float deltaTime) {};

protected:
	Tickable(bool add_to_tick_list = true)
	    : TIntrusiveLinkedList<Tickable>(add_to_tick_list)
	{
	}
};