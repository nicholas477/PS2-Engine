#pragma once
#include "tamtypes.h"
#include "renderer/gs.hpp"
#include "utils/list.hpp"

class renderable: public TIntrusiveLinkedList<renderable>
{
public:
	virtual void on_gs_init() {};
	virtual void render(const gs::gs_state& gs_state) {};

protected:
	renderable(bool add_to_render_list = true)
	    : TIntrusiveLinkedList<renderable>(add_to_render_list)
	{
	}
};