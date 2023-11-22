#pragma once
#include "utils/list.hpp"

// This uses an intrusive linked list since objects can be created/deleted during tick
// If we used an array of pointers it would be invalidated
class tickable: public TIntrusiveLinkedList<tickable>
{
public:
	virtual void tick(float deltaTime) {};

protected:
	tickable(bool add_to_tick_list = true)
	    : TIntrusiveLinkedList<tickable>(add_to_tick_list)
	{
	}
};