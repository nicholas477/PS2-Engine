#include "input.hpp"
#include "world.hpp"
#include "renderer/mesh.hpp"
#include "objects/teapot.hpp"
#include "objects/camera.hpp"
#include "objects/movement.hpp"
#include "objects/components/collision_component.hpp"

#include "timer.h"

#include <debug.h>

static void make_teapot()
{
	static int i       = 0;
	teapot* new_teapot = new teapot();
	srand(GetTimerSystemTime() + i++);
	new_teapot->transform.add_location(Vector(((double)rand() / (double)RAND_MAX) * 256.f, 0.f, ((double)rand() / (double)RAND_MAX) * 256.f));
}

namespace world
{
void init()
{
	// Other mesh
	// static struct other_mesh: public renderable
	// {
	// public:
	// 	virtual void on_gs_init() override
	// 	{
	// 		//mesh = &Mesh::loaded_meshes["/assets/models/shopping_cart.ps2_model"_p];
	// 	}
	// 	virtual void render(const gs::gs_state& gs_state) override
	// 	{
	// 		const Matrix local_world = Matrix::from_location_and_rotation(transform.get_location(), transform.get_rotation());

	// 		ScopedMatrix sm(local_world * gs_state.world_view);

	// 		static float ps2_diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
	// 		static float black[]       = {0, 0, 0, 0};
	// 		//glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, ps2_diffuse);
	// 		//glMaterialfv(GL_FRONT, GL_EMISSION, black);

	// 		//mesh->draw();
	// 	}
	// 	transform_component transform;
	// 	Mesh* mesh;
	// } other_mesh;
	// other_mesh.transform.set_location(Vector(100.f, 20.f));

	// Player teapot
	static teapot teapot4;
	teapot4.transform.set_location(Vector(0.f, 0.f, 0.f));


	// Camera parent component
	static struct camera_transform_component: public transform_component, public tickable
	{
		virtual void tick(float deltatime) override
		{
			const u32 paddata   = input::get_paddata();
			const auto& buttons = input::get_button_status();

			Vector input_vector = Vector::zero;

			input_vector.y -= (buttons.l2_p - 128.f) / 128.f;
			input_vector.y += (buttons.r2_p - 128.f) / 128.f;

			add_location(input_vector * 20 * deltatime);
		}
	} scene;

	scene.set_parent(teapot4.transform);
	scene.set_location(Vector(0.f, teapot4.collision.get_local_bounds().get_half_extents().y));

	camera::get().transform.set_parent(scene);

	static third_person_movement movement;
	movement.updated_rotation_component = &scene;
	movement.updated_location_component = &teapot4.transform;
	movement.collision_component        = &teapot4.collision;
}
} // namespace world