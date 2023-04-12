#pragma once

#include "types.hpp"

#include <set>


class transform_component
{
public:
	Matrix get_matrix() const;
	Matrix get_rotation_matrix() const;

	Vector get_forward_vector() const;
	Vector get_right_vector() const;

	const Vector& get_location() const { return location; }
	const Vector& get_rotation() const { return rotation; }

	void add_location(const Vector& offset) { location += offset; }
	void add_rotation(const Vector& offset)
	{
		rotation += offset;
		rotation = rotation.normalize_euler_rotation();
	}

	void set_location(const Vector& new_location) { location = new_location; }
	void set_rotation(const Vector& new_rotation) { rotation = new_rotation; }

	void set_parent(transform_component& new_parent)
	{
		parent = &new_parent;
		new_parent.children.insert(this);
	}
	transform_component* get_parent() const { return parent; }

protected:
	Vector location;
	Vector rotation;

	transform_component* parent = nullptr;
	std::set<transform_component*> children;
};