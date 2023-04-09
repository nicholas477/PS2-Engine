#include "transform_component.h"

Matrix transform_component::get_rotation_matrix() const
{
	Matrix out_matrix;
	// Create the local_light matrix.
	matrix_unit(out_matrix.matrix);
	matrix_rotate(out_matrix.matrix, out_matrix.matrix, const_cast<float*>(rotation.vector));
	return out_matrix;
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
