#pragma once
#include "utils/list.hpp"

// This uses an intrusive linked list since objects can be created/deleted during tick
// If we used an array of pointers it would be invalidated
class tickable: public TIntrusiveLinkedList<tickable>
{
public:
	tickable()
	{
		if (head == nullptr)
		{
			head = this;
		}
		else
		{
			LinkHead(head);
		}
	}

	virtual void tick(float deltaTime) {};

	virtual ~tickable()
	{
		if (head == this)
		{
			head = Next();
		}

		this->Unlink();
	}

	static tickable::TIterator Itr()
	{
		return tickable::TIterator(head);
	}

	static tickable::TConstIterator ConstItr()
	{
		return tickable::TConstIterator(head);
	}

protected:
	// head of the tickables list
	static tickable* head;
};