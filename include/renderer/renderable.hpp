#pragma once
#include "tamtypes.h"
#include "renderer/gs.hpp"

class renderable
{
public:
	renderable()
	{
		gs::add_renderable(this);
	};
	virtual qword_t* render(qword_t* q, const gs::gs_state& gs_state) { return q; };
};