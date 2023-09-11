#include "objects/components/collision_component.hpp"

#include "objects/components/transform_component.hpp"
#include "utils/rendering.hpp"
#include "collision/plane.hpp"
#include "objects/timer.hpp"

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

void collision_component::render(const gs::gs_state& gs_state, color_t color)
{
	draw_aabb(gs_state, get_world_bounds(), color);
}

hit_result collideable::sweep_collision(const collideable& collideable, const Matrix& sweep_start, const Matrix& sweep_end)
{
	hit_result out_result;
	out_result.hit = false;

	// Check plane intersection
	Vector center, extents;
	collideable.get_local_bounds().get_center_and_half_extents(center, extents);
	center.w = 0;

	const Vector start = center + sweep_start.get_location();
	const Vector end   = center + sweep_end.get_location();

	for (auto Itr = collideable::Itr(); Itr; ++Itr)
	{
		// Don't check collision against self obv
		if (&*Itr == &collideable)
		{
			continue;
		}
		const AABB other_aabb    = (*Itr).get_world_bounds();
		const AABB minkowski_sum = other_aabb.minkowski_sum(collideable.get_local_bounds());
		//minkowski_sum.check_valid();
		//draw_aabb_one_frame(minkowski_sum, colors::white(), false);

		std::array<Plane, 6> planes = minkowski_sum.get_face_planes();

		AABB_face intersected_face;
		bool found_intersection = false;
		Vector intersect_vector = Vector::zero;
		for (u8 i = 0; i < 6; ++i)
		{
			AABB_face face = int_to_aabb_face(i);
			Plane& plane   = planes[i];

			// Make sure the line formed by the edge actually crosses the plane before checking for the intersection point.
			if (plane.line_crosses_plane(start, end))
			{
				Vector intersect = line_plane_intersection(start, end, plane);
				if (minkowski_sum.point_on_face(intersect, face))
				{
					if (!found_intersection)
					{
						found_intersection = true;
						intersect_vector   = intersect;
						intersected_face   = face;
					}
					else
					{
						// If we already hit a plane face, then check if we've found a closer intersection
						float old_intersection_distance = start.distance(intersect_vector);
						float new_intersection_distance = start.distance(intersect);

						if (new_intersection_distance < old_intersection_distance)
						{
							intersect_vector = intersect;
							intersected_face = face;
						}
					}
				}
			}
		}

		out_result.hit = found_intersection;
		if (out_result.hit)
		{
			out_result.sweep_location = intersect_vector - center;

			const Plane& hit_plane = planes[aabb_face_to_int(intersected_face)];

			// Check that the offsetted result doesn't put the object behind the plane
			while (hit_plane.plane_dot(out_result.sweep_location + center) < 0.f)
			{
				Vector push_direction = (start - end).normalize();

				// Make sure we're nudging AWAY from the plane
				// If not, then just use the plane normal for nudge direction
				if ((hit_plane.dot(push_direction) <= 0.f))
				{
					push_direction = hit_plane.get_normal();
				}

				check(hit_plane.dot(push_direction) > 0.f);

				// Nudge it back the way it came
				out_result.sweep_location += push_direction * 0.01f;
			}
		}
	}

	return out_result;
}