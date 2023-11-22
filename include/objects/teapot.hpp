#pragma once

#include "tick.hpp"
#include "renderer/renderable.hpp"
#include "components/transform_component.hpp"
#include "components/collision_component.hpp"

class Teapot: public Renderable
{
public:
	Teapot();
	virtual void render(const GS::GSState& gs_state) override;
	TransformComponent transform;
	collision_component collision;

	class teapot_render_proxy* render_proxy;
};