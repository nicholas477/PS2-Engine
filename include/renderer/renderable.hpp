#pragma once
#include "tamtypes.h"
#include "renderer/gs.hpp"
#include "utils/list.hpp"

class renderable: public TIntrusiveLinkedList<renderable>
{
public:
	virtual void on_gs_init() {};
	virtual void render(const gs::gs_state& gs_state) {};

	virtual void add_to_renderables_list()
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

	virtual void remove_from_renderables_list()
	{
		if (head == this)
		{
			head = Next();
		}

		this->Unlink();
	}

protected:
	renderable(bool add_to_render_list = true)
	    : TIntrusiveLinkedList<renderable>(add_to_render_list)
	{
	}
};