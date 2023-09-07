#include "objects/components/collision_component.hpp"

#include "objects/components/transform_component.hpp"
#include "utils/rendering.hpp"

collideable* collideable::head = nullptr;

AABB collision_component::get_world_bounds() const
{
	if (transform)
	{
		const Matrix TransformMatrix = transform->get_matrix();
		return Bounds.transform(TransformMatrix);
	}
	else
	{
		return Bounds;
	}
}

qword_t* collision_component::render(qword_t* q, const gs::gs_state& gs_state, color_t color)
{
	return draw_aabb(q, gs_state, get_world_bounds(), color);
}

hit_result collideable::sweep_collision(const collideable& collideable, Matrix sweep_start, Matrix sweep_end)
{
	hit_result out_result;

	const AABB sweep_end_aabb = collideable.get_local_bounds().transform(sweep_end);

	for (auto Itr = collideable::Itr(); Itr; ++Itr)
	{
		// Don't check collision against self obv
		if (&*Itr == &collideable)
		{
			continue;
		}
		const AABB other_aabb = (*Itr).get_world_bounds();

		// Right now just check for intersection, nothing else
		if (sweep_end_aabb.intersect(other_aabb))
		{
			out_result.hit_collideable = &*Itr;
			out_result.hit             = true;
			return out_result;
		}
	}

	return out_result;
}