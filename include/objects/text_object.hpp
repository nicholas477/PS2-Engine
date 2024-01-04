#pragma once
#include "renderer/renderable.hpp"
#include "utils/debuggable.hpp"
#include "components/transform_component.hpp"


class TextObject: public TextRenderable, public Debuggable
{
public:
	TextObject();
	TextObject(std::string_view in_text);

	virtual void render(const GS::GSState& gs_state) override;
	void draw(bool flush = false);

	TransformComponent transform;

	std::string text;
	virtual const char* get_name() const override;
	virtual const char* get_type_name() const { return typeid(TextObject).name(); }
};