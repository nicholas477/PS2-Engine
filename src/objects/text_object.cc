#include "objects/text_object.hpp"

#include "renderer/text.hpp"

TextObject::TextObject()
{
	text = "";
}

TextObject::TextObject(std::string_view in_text)
{
	text = in_text;
}

void TextObject::render(const GS::GSState& gs_state)
{
	if (text.size() > 0)
	{
		Vector screen_pos;
		if (gs_state.world_to_screen(transform.get_location(), screen_pos))
		{
			float material[] = {1.0f, 1.0f, 1.0f, 1.0f};
			glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);

			tsDrawString(screen_pos.x, screen_pos.y, text.c_str());
		}
	}
}

void TextObject::set_text(std::string_view new_text)
{
	text = new_text;
}

std::string_view TextObject::get_text() const
{
	return text;
}

const char* TextObject::get_name() const
{
	if (text.size() > 0)
	{
		return text.c_str();
	}
	else
	{
		return "EMPTY TEXT OBJECT";
	}
}