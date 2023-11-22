#include "objects/components/transform_component.hpp"

TransformComponent::TransformComponent()
{
	location = Vector::zero;
	rotation = Vector::zero;
	scale    = Vector(1.f, 1.f, 1.f);
}

Matrix TransformComponent::get_matrix() const
{
	if (parent)
	{
		return Matrix::from_location_and_rotation(location, rotation) * Matrix::from_scale(scale) * parent->get_matrix();
	}
	else
	{
		return Matrix::from_location_and_rotation(location, rotation) * Matrix::from_scale(scale);
	}
}

Matrix TransformComponent::get_rotation_matrix() const
{
	if (parent)
	{
		return rotation.to_rotation_matrix() * parent->get_rotation_matrix();
	}
	else
	{
		return rotation.to_rotation_matrix();
	}
}

Vector TransformComponent::get_forward_vector() const
{
	Vector out_vector = Vector(0.f, 0.f, 1.f);
	return get_rotation_matrix().transform_vector(out_vector);
}

Vector TransformComponent::get_right_vector() const
{
	Vector out_vector = Vector(1.f, 0.f, 0.f);
	return get_rotation_matrix().transform_vector(out_vector);
}

Vector TransformComponent::get_up_vector() const
{
	Vector out_vector = Vector(0.f, 1.f, 0.f);
	return get_rotation_matrix().transform_vector(out_vector);
}
