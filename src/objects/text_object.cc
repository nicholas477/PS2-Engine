#include "objects/text_object.hpp"

#include "renderer/text.hpp"
#include "renderer/helpers.hpp"

TextObject::TextObject()
{
	text                   = "";
	quad_background_offset = Vector::zero;
	text_screen_offset     = Vector2::zero;
}

TextObject::TextObject(std::string_view in_text)
{
	text                   = in_text;
	quad_background_offset = Vector::zero;
	text_screen_offset     = Vector2::zero;
}

void TextObject::render(const GS::GSState& gs_state)
{
	if (text.size() > 0)
	{
		Vector screen_pos;
		if (gs_state.world_to_screen(transform.get_location(), screen_pos))
		{
			printf("############rendering quad!\n");
			IntVector2 text_size = get_text_size();
			GS::Helpers::draw_2d_quad(Vector(screen_pos.x, screen_pos.y, text_size.x, text_size.y) + quad_background_offset, Colors::black());

			printf("############rendering text!\n");
			float material[] = {1.0f, 1.0f, 1.0f, 1.0f};
			glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material);
			tsDrawString(screen_pos.x + text_screen_offset.x, screen_pos.y + text_screen_offset.y, text.c_str());
		}
	}
}

void TextObject::set_text(std::string_view new_text)
{
	text = new_text;
}

IntVector2 TextObject::get_text_size() const
{
	IntVector2 cursor_max = {0, 0};
	IntVector2 cursor     = {0, 0};
	for (size_t i = 0; i < text.size(); ++i)
	{
		if (text[i] == '\n')
		{
			cursor.x = 0;
			cursor.y += 16;
		}
		else
		{
			cursor.x += 8;
		}
		cursor_max.x = std::max(cursor_max.x, cursor.x);
		cursor_max.y = std::max(cursor_max.y, cursor.y);
	}

	cursor_max.x += 8;
	cursor_max.y += 16;
	return cursor_max;
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