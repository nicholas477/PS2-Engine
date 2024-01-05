#pragma once
#include "objects/text_object.hpp"
#include "tick.hpp"

class TextPrompt: public Tickable, public RootComponentInterface
{
public:
	std::string prompt;
	std::string inputted_text;

	// Actual 3d text rendered
	TextObject text_object;

	virtual void tick(float deltaTime) override;
	virtual TransformComponent* get_root_component() override { return text_object.get_root_component(); }

	void set_prompt(std::string_view new_prompt);
};