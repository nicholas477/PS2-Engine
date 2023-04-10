#pragma once

#include "tick.hpp"
#include "renderable.hpp"
#include "components/transform_component.hpp"

class teapot: public renderable
{
public:
	teapot();
	virtual qword_t* render(qword_t* q, const gs::gs_state& gs_state) override;
	transform_component transform;

	class teapot_render_proxy* render_proxy;
};