#pragma once

#include "tick.hpp"
#include "renderer/renderable.hpp"
#include "components/transform_component.hpp"
#include "components/collision_component.hpp"

class teapot: public renderable
{
public:
	teapot();
	virtual void render(const gs::gs_state& gs_state) override;
	transform_component transform;
	collision_component collision;

	class teapot_render_proxy* render_proxy;
};