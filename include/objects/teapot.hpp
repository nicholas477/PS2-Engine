#pragma once

#include "tick.hpp"
#include "renderer/renderable.hpp"
#include "utils/debuggable.hpp"
#include "components/transform_component.hpp"
#include "components/collision_component.hpp"

class Teapot: public Renderable, public Debuggable
{
public:
	Teapot();
	virtual void render(const GS::GSState& gs_state) override;
	TransformComponent transform;
	collision_component collision;

	class teapot_render_proxy* render_proxy;

	virtual const char* get_type_name() const override { return typeid(Teapot).name(); }
};