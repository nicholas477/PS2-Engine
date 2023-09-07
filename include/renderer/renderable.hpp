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
	[[nodiscard]] virtual qword_t* render(qword_t* q, const gs::gs_state& gs_state) { return q; };
};