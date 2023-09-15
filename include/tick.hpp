#pragma once
#include "utils/list.hpp"

// This uses an intrusive linked list since objects can be created/deleted during tick
// If we used an array of pointers it would be invalidated
class tickable: public TIntrusiveLinkedList<tickable>
{
public:
	virtual void tick(float deltaTime) {};
};