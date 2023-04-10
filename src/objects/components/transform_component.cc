#include "objects/components/transform_component.hpp"

Matrix transform_component::get_matrix() const
{
	if (parent)
	{
		return parent->get_matrix() * Matrix::from_location_and_rotation(location, rotation);
	}
	else
	{
		return Matrix::from_location_and_rotation(location, rotation);
	}
}

Matrix transform_component::get_rotation_matrix() const
{
	return rotation.to_rotation_matrix();
}

Vector transform_component::get_forward_vector() const
{
	Vector out_vector = Vector(0.f, 0.f, 1.f);
	return get_rotation_matrix().transform_vector(out_vector);
}

Vector transform_component::get_right_vector() const
{
	Vector out_vector = Vector(1.f, 0.f, 0.f);
	return get_rotation_matrix().transform_vector(out_vector);
}
