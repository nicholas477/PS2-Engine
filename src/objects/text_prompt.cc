#include "objects/text_prompt.hpp"

#include "input/keyboard.hpp"

#include <stdio.h>
#include <ctype.h>

void TextPrompt::tick(float deltaTime)
{
	using namespace Input::Keyboard;

	bool new_text = false;

	const auto& keyboard_keys = get_all_key_statuses();
	for (int i = 0; i < 256; ++i)
	{
		if (keyboard_keys[i] == KeyStatus::pressed)
		{
			const unsigned char ascii_key = convert_keyboard_key_to_ascii(i);
			if (isalpha(ascii_key))
			{
				inputted_text += ascii_key;
				new_text = true;
			}
			else if (ascii_key == '.' || ascii_key == ',')
			{
				inputted_text += ascii_key;
				new_text = true;
			}
			else if (i == 56 && is_shift_down())
			{
				inputted_text += '?';
				new_text = true;
			}
			else if (i == 44) // space bar
			{
				inputted_text += ' ';
				new_text = true;
			}
			else if (i == 42 && inputted_text.size() > 0) // delete key
			{
				inputted_text.pop_back();
				new_text = true;
			}
		}
	}

	if (new_text)
	{
		text_object.set_text(prompt + "\n" + inputted_text);
	}
}

void TextPrompt::set_prompt(std::string_view new_prompt)
{
	prompt = new_prompt;
	text_object.set_text(prompt + "\n" + inputted_text);
}