#pragma once

#include "utils/list.hpp"
#include "collision/AABB.hpp"
#include "renderer/renderable.hpp"

#include "draw_types.h"

struct hit_result
{
	bool hit = false;

	// Where the sweep stopped
	Vector sweep_location              = Vector::zero;
	class collideable* hit_collideable = nullptr;
};

class collideable //: public TIntrusiveLinkedList<collideable>
{
public:
	virtual AABB get_world_bounds() const = 0;
	virtual AABB get_local_bounds() const = 0;

	static hit_result sweep_collision(const collideable& collideable, const Matrix& sweep_start, const Matrix& sweep_end);

	// protected:
	// 	collideable()
	// 	    : TIntrusiveLinkedList<collideable>(true)
	// 	{
	// 	}
};

class collision_component //: public collideable
{
public:
	collision_component() = default;
	collision_component(class TransformComponent* in_transform)
	    : transform(in_transform)
	{
	}

	class TransformComponent* transform;

	virtual AABB get_world_bounds() const;
	virtual AABB get_local_bounds() const { return Bounds; }
	virtual void render(const GS::GSState& gs_state, color_t color);

	void set_local_bounds(const AABB& in_bounds) { Bounds = in_bounds; }

protected:
	// Bounds (in local space)
	AABB Bounds;
};