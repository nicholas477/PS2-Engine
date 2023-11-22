#pragma once

#include "egg/math_types.hpp"

#include <set>


class TransformComponent
{
public:
	TransformComponent();

	Matrix get_matrix() const;
	Matrix get_rotation_matrix() const;

	Vector get_forward_vector() const;
	Vector get_right_vector() const;
	Vector get_up_vector() const;

	const Vector& get_location() const { return location; }
	const Vector& get_rotation() const { return rotation; }

	void add_location(const Vector& offset) { location += offset; }
	void add_rotation(const Vector& offset)
	{
		rotation += offset;
		rotation = rotation.clamp_euler_rotation();
	}

	void set_location(const Vector& new_location) { location = new_location; }
	void set_rotation(const Vector& new_rotation) { rotation = new_rotation; }

	void set_parent(TransformComponent& new_parent)
	{
		parent = &new_parent;
		new_parent.children.insert(this);
	}
	TransformComponent* get_parent() const { return parent; }

protected:
	Vector location;
	Vector rotation;
	Vector scale;

	TransformComponent* parent = nullptr;
	std::set<TransformComponent*> children;
};