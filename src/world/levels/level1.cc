#include "world/level.hpp"
#include "world/world.hpp"

#include "objects/teapot.hpp"
#include "objects/camera.hpp"
#include "objects/movement.hpp"
#include "objects/text_prompt.hpp"
#include "objects/mesh_object.hpp"

#include "input/gamepad.hpp"

class Level1Player
{
public:
	Level1Player()
	    : teeth_model("assets/models/level1/Mouth.mdl"_asset)
	{
		camera_transform.set_parent(teapot_model.transform);
		camera_transform.set_location(Vector(0.f, teapot_model.collision.get_local_bounds().get_half_extents().y));

		Camera::get().transform.set_parent(camera_transform);

		movement.updated_rotation_component = &camera_transform;
		movement.updated_location_component = &teapot_model.transform;
		movement.collision_component        = &teapot_model.collision;

		teeth_model.get_root_component()->set_parent(Camera::get().transform);
	}

	Teapot teapot_model;
	MeshObject teeth_model;
	ThirdPersonMovement movement;

	struct: public TransformComponent,
	        public Tickable
	{
		virtual void tick(float deltatime) override
		{
			const u32 paddata   = Input::Gamepad::get_paddata();
			const auto& buttons = Input::Gamepad::get_button_status();

			Vector input_vector = Vector::zero;

			input_vector.y -= (buttons.l2_p - 128.f) / 128.f;
			input_vector.y += (buttons.r2_p - 128.f) / 128.f;

			add_location(input_vector * 20 * deltatime);
		}
	} camera_transform;
};

class Level1: public World::Level
{
public:
	Level1(Asset::Reference level)
	    : World::Level(level)
	{
		// Player teapot
		player.teapot_model.transform.set_location(Vector(0.f, 0.f, 0.f));

		t1.get_root_component()->set_location(Vector(100.f, 0.f, 0.f));
		t1.set_prompt("Cream of mushroom");
	}

	virtual void initialize() override
	{
	}

	static Level* create(Asset::Reference level_ref)
	{
		return new Level1(level_ref);
	}

	Level1Player player;
	TextPrompt t1;
};

static struct Level1ConstructorHelper
{
	Level1ConstructorHelper()
	{
		World::get()->register_level_constructor("assets/levels/new_level.lvl"_asset, &Level1::create);
	}
} _level1_constructor_helper;