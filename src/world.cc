#include "world.hpp"
#include "objects/teapot.hpp"
#include "objects/camera.hpp"
#include "objects/movement.hpp"

namespace world
{
void init()
{
	static teapot teapot1;
	teapot1.transform.set_location(Vector(30.f));

	static teapot teapot2;
	teapot2.transform.set_location(Vector(-30.f));

	static teapot teapot3;
	teapot3.transform.set_location(Vector(0.f, 20.f));

	static teapot teapot4;
	teapot4.transform.set_location(Vector(0.f, -20.f));

	static transform_component scene;
	scene.set_parent(teapot4.transform);
	scene.set_location(Vector(0.f, 50.f));

	camera::get().transform.set_parent(scene);

	static third_person_movement movement;
	movement.updated_rotation_component = &scene;
	movement.updated_location_component = &teapot4.transform;
}
} // namespace world