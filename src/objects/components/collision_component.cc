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

qword_t* collision_component::render(qword_t* q, const gs::gs_state& gs_state, color_t color)
{
	return draw_aabb(q, gs_state, get_world_bounds(), color);
}

hit_result collideable::sweep_collision(const collideable& collideable, const Matrix& sweep_start, const Matrix& sweep_end)
{
	hit_result out_result;

	// Check plane intersection
	Vector center, extents;
	collideable.get_local_bounds().get_center_and_half_extents(center, extents);
	//center.w = center.x = center.z = 0;
	center.w = 0;

	// printf("center: %s\n", center.to_string().c_str());

	const Vector start = center + sweep_start.get_location();
	const Vector end   = center + sweep_end.get_location();

	// printf("start: %s\n", start.to_string().c_str());
	// printf("end: %s\n", end.to_string().c_str());

	draw_point_one_frame(start, 10, colors::white(), true);
	draw_point_one_frame(end, 10, colors::black(), true);

	for (auto Itr = collideable::Itr(); Itr; ++Itr)
	{
		// Don't check collision against self obv
		if (&*Itr == &collideable)
		{
			continue;
		}
		const AABB other_aabb    = (*Itr).get_world_bounds();
		const AABB minkowski_sum = other_aabb.minkowski_sum(collideable.get_local_bounds());
		draw_aabb_one_frame(minkowski_sum, colors::white(), false);

		std::array<Plane, 6> planes;

		// X min
		planes[static_cast<u8>(AABB::AABB_face::x_neg)] = Plane(minkowski_sum.Min, Vector(-1, 0, 0));
		// X max
		planes[static_cast<u8>(AABB::AABB_face::x_pos)] = Plane(minkowski_sum.Max, Vector(1, 0, 0));

		// Y min
		planes[static_cast<u8>(AABB::AABB_face::y_neg)] = Plane(minkowski_sum.Min, Vector(0, -1, 0));
		// Y max
		planes[static_cast<u8>(AABB::AABB_face::y_pos)] = Plane(minkowski_sum.Max, Vector(0, 1, 0));

		// Z min
		planes[static_cast<u8>(AABB::AABB_face::z_neg)] = Plane(minkowski_sum.Min, Vector(0, 0, -1));
		// Z max
		planes[static_cast<u8>(AABB::AABB_face::z_pos)] = Plane(minkowski_sum.Max, Vector(0, 0, 1));


		bool found_intersection = false;
		Vector intersect_vector = Vector::zero;
		for (u8 i = 0; i < 6; ++i)
		{
			AABB::AABB_face face = static_cast<AABB::AABB_face>(i);
			const Plane& plane   = planes[i];

			// Make sure the line formed by the edge actually crosses the plane before checking for the intersection point.
			if (plane.line_crosses_plane(start, end))
			{
				create_managed_timer_lambda(5.f, [=](timer*, float) {
					draw_plane_one_frame(plane, 100, colors::green(), true);
					draw_line_one_frame(start, end, colors::green(), true);
					draw_aabb_one_frame(minkowski_sum.get_face_aabb(face), colors::green(), true);
					draw_point_one_frame(start, 10, colors::blue(), true);
					draw_point_one_frame(end, 10, colors::blue(), true);
				});

				Vector intersect = line_plane_intersection(start, end, plane);
				if (minkowski_sum.point_on_face(intersect, face))
				{

					if (!found_intersection)
					{
						found_intersection = true;
						intersect_vector   = intersect;
					}
					else
					{
						// If we already hit a plane face, then check if we've found a closer intersection
						float old_intersection_distance = start.distance(intersect_vector);
						float new_intersection_distance = start.distance(intersect);

						if (new_intersection_distance < old_intersection_distance)
						{
							intersect_vector = intersect;
						}
					}
				}
			}
		}

		out_result.hit = found_intersection;
		if (out_result.hit)
		{
			out_result.sweep_location = intersect_vector - center;
		}
	}

	return out_result;
}