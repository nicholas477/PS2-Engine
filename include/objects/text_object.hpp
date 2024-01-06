#pragma once
#include "renderer/renderable.hpp"
#include "utils/debuggable.hpp"
#include "components/transform_component.hpp"

class TextObject: public TextRenderable, public RootComponentInterface, public Debuggable
{
public:
	TextObject();
	TextObject(std::string_view in_text);

	virtual void render(const GS::GSState& gs_state) override;
	void draw(bool flush = false);

	TransformComponent transform;
	virtual TransformComponent* get_root_component() override { return &transform; }

	virtual void set_text(std::string_view new_text);
	std::string_view get_text() const;

	virtual const char* get_name() const override;
	virtual const char* get_type_name() const { return typeid(TextObject).name(); }

	// Calculates the maximum width and height of the text if it were drawn
	IntVector2 get_text_size() const;

	Vector quad_background_offset;
	Vector2 text_screen_offset;

protected:
	std::string text;
};