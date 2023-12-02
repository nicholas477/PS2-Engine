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

#include "timer.h"

#include <debug.h>

static void make_teapot()
{
	static int i       = 0;
	Teapot* new_teapot = new Teapot();
	srand(GetTimerSystemTime() + i++);
	new_teapot->transform.add_location(Vector(((double)rand() / (double)RAND_MAX) * 256.f, 0.f, ((double)rand() / (double)RAND_MAX) * 256.f));
}

class WorldMesh: public Renderable
{
public:
	WorldMesh(const Filesystem::Path& p)
	{
		Filesystem::load_file(p, level_data, level_size);
		level = (LevelFileHeader*)level_data.get();

		collect_references(level_references, level->meshes);

		for (Asset::Reference ref : level_references)
		{
			const Filesystem::Path& p = Asset::lookup_path(ref);

			printf("Loading mesh: %s\n", p.data());

			meshes[p] = Mesh(p);
			meshes[p].compile();
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
};

namespace World
{
void init()
{
	static WorldMesh world("assets/new_level.lvl"_p);

	// Player teapot
	static Teapot teapot4;
	teapot4.transform.set_location(Vector(0.f, 0.f, 0.f));

	//static MeshObject haid_mesh("assets/models/haid_mes/Brick_WallBake.mdl"_p);
	//haid_mesh.mesh->compile();


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
}
} // namespace World