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

class collideable: public TIntrusiveLinkedList<collideable>
{
public:
	virtual AABB get_world_bounds() const = 0;
	virtual AABB get_local_bounds() const = 0;

	virtual ~collideable()
	{
		if (head == this)
		{
			head = Next();
		}

		this->Unlink();
	};

	static collideable::TIterator Itr()
	{
		return collideable::TIterator(head);
	}

	static collideable::TConstIterator ConstItr()
	{
		return collideable::TConstIterator(head);
	}

	static hit_result sweep_collision(const collideable& collideable, const Matrix& sweep_start, const Matrix& sweep_end);

protected:
	collideable()
	{
		if (head == nullptr)
		{
			head = this;
		}
		else
		{
			LinkHead(head);
		}
	}

private:
	static collideable* head;
};

class collision_component: public collideable
{
public:
	collision_component() = default;
	collision_component(class transform_component* in_transform)
	    : transform(in_transform)
	{
	}

	class transform_component* transform;

	virtual AABB get_world_bounds() const;
	virtual AABB get_local_bounds() const { return Bounds; }
	virtual void render(const gs::gs_state& gs_state, color_t color);

	void set_local_bounds(const AABB& in_bounds) { Bounds = in_bounds; }

protected:
	// Bounds (in local space)
	AABB Bounds;
};