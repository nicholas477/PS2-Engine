#include "objects/components/collision_component.hpp"

#include "objects/components/transform_component.hpp"
#include "utils/rendering.hpp"
#include "collision/plane.hpp"

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

hit_result collideable::sweep_collision(const collideable& collideable, const Matrix& sweep_start, const Matrix& sweep_end)
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
		const AABB other_aabb    = (*Itr).get_world_bounds();
		const AABB minkowski_sum = other_aabb.minkowski_sum(sweep_end_aabb);

		std::array<Plane, 6> planes;

		// X min
		planes[0] = Plane(minkowski_sum.Min, Vector(-1, 0, 0));
		// X max
		planes[1] = Plane(minkowski_sum.Max, Vector(1, 0, 0));

		// Y min
		planes[2] = Plane(minkowski_sum.Min, Vector(0, -1, 0));
		// Y max
		planes[3] = Plane(minkowski_sum.Max, Vector(0, 1, 0));

		// Z min
		planes[4] = Plane(minkowski_sum.Min, Vector(0, 0, -1));
		// Z max
		planes[5] = Plane(minkowski_sum.Max, Vector(0, 0, 1));

		for (const Plane& plane : planes)
		{
			//draw_plane_one_frame(plane, 50, colors::blue());
			draw_point_one_frame(plane.get_origin(), 5, colors::blue());
		}


		draw_aabb_one_frame(minkowski_sum, colors::red());
		draw_line_one_frame(sweep_start.get_location(), sweep_end.get_location(), colors::green());

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