#pragma once
#include "tamtypes.h"
#include "renderer/gs.hpp"

class renderable
{
public:
	renderable(bool bAddToRenderList = true)
	{
		if (bAddToRenderList)
		{
			gs::add_renderable(this);
		}
	};
	virtual void render(const gs::gs_state& gs_state) {};
};