#include "input.hpp"
#include "world.hpp"
#include "renderer/mesh.hpp"
#include "objects/teapot.hpp"
#include "objects/camera.hpp"
#include "objects/movement.hpp"
#include "objects/components/collision_component.hpp"
#include "objects/mesh_object.hpp"

#include "egg/level.hpp"
#include "egg/mesh_header.hpp"
#include "egg/asset.hpp"

#include "timer.h"

#include <debug.h>

static void make_teapot()
{
	static int i       = 0;
	Teapot* new_teapot = new Teapot();
	srand(GetTimerSystemTime() + i++);
	new_teapot->transform.add_location(Vector(((double)rand() / (double)RAND_MAX) * 256.f, 0.f, ((double)rand() / (double)RAND_MAX) * 256.f));
}

class WorldMesh: public Renderable, public Debuggable
{
public:
	WorldMesh(Asset::Reference world_reference)
	{
		checkf(Filesystem::load_file(world_reference, level_data, level_size), "unable to load level!");
		debug_name = Asset::lookup_path(world_reference).data();
		level      = (LevelFileHeader*)level_data.get();

		collect_references(level_references, level->meshes);

		for (Asset::Reference ref : level_references)
		{
			meshes[ref] = Mesh(ref);

			checkf(meshes[ref].is_valid(), "Mesh invalid!");

			printf("\n");
		}
	}

	virtual void render(const GS::GSState& gs_state) override
	{
		for (int i = 0; i < level->meshes.mesh_files.num_elements(); ++i)
		{
			ScopedMatrix m(level->meshes.mesh_transforms[i]);
			meshes[level->meshes.mesh_files[i]].draw();
		}
	}

	size_t level_size;
	std::unique_ptr<std::byte[]> level_data;
	LevelFileHeader* level;

	std::unordered_set<Asset::Reference> level_references;
	std::unordered_map<Asset::Reference, Mesh> meshes;

	virtual const char* get_type_name() const { return typeid(WorldMesh).name(); }
};

namespace World
{
void init()
{
	printf("Initializing world\n");
	static WorldMesh world("assets/levels/new_level.lvl"_asset);

	// Player teapot
	static Teapot teapot4;
	teapot4.transform.set_location(Vector(0.f, 0.f, 0.f));


	// Camera parent component
	static struct camera_transform_component: public TransformComponent,
	                                          public Tickable
	{
		virtual void tick(float deltatime) override
		{
			const u32 paddata   = Input::get_paddata();
			const auto& buttons = Input::get_button_status();

			Vector input_vector = Vector::zero;

			input_vector.y -= (buttons.l2_p - 128.f) / 128.f;
			input_vector.y += (buttons.r2_p - 128.f) / 128.f;

			add_location(input_vector * 20 * deltatime);
		}
	} scene;

	scene.set_parent(teapot4.transform);
	scene.set_location(Vector(0.f, teapot4.collision.get_local_bounds().get_half_extents().y));

	Camera::get().transform.set_parent(scene);

	static ThirdPersonMovement movement;
	movement.updated_rotation_component = &scene;
	movement.updated_location_component = &teapot4.transform;
	movement.collision_component        = &teapot4.collision;

	printf("World initialized!\n");
}
} // namespace World