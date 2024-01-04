#pragma once
#include "tamtypes.h"
#include "renderer/gs.hpp"
#include "utils/list.hpp"

class Renderable: public TIntrusiveLinkedList<Renderable>
{
public:
	virtual void on_gs_init() {};
	virtual void render(const GS::GSState& gs_state) {};

protected:
	Renderable(bool add_to_render_list = true)
	    : TIntrusiveLinkedList<Renderable>(add_to_render_list)
	{
	}
};

class TextRenderable: public TIntrusiveLinkedList<TextRenderable>
{
public:
	virtual void on_gs_init() {};
	virtual void render(const GS::GSState& gs_state) {};

protected:
	TextRenderable(bool add_to_render_list = true)
	    : TIntrusiveLinkedList<TextRenderable>(add_to_render_list)
	{
	}
};