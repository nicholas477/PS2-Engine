#pragma once
#include "objects/text_object.hpp"
#include "tick.hpp"

#include "egg/asset.hpp"

class TextPrompt: public Tickable, public RootComponentInterface
{
public:
	TextPrompt();
	std::string prompt;
	std::string inputted_text;
	Asset::Reference key_sounds[3];
	Asset::Reference finish_sound;
	uint32_t cursor;

	// Actual 3d text rendered
	TextObject text_object;

	virtual void tick(float deltaTime) override;
	virtual TransformComponent* get_root_component() override { return text_object.get_root_component(); }

	void set_prompt(std::string_view new_prompt);
};