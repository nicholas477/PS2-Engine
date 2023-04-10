#include "objects/components/transform_component.hpp"

Matrix transform_component::get_rotation_matrix() const
{
	MATRIX work;

	MATRIX out_matrix;
	matrix_unit(out_matrix);

	const VECTOR& input1 = rotation.vector;

	// Apply the x-axis rotation.
	matrix_unit(work);
	work[0x05] = cosf(input1[0]);
	work[0x06] = sinf(input1[0]);
	work[0x09] = -sinf(input1[0]);
	work[0x0A] = cosf(input1[0]);
	matrix_multiply(out_matrix, out_matrix, work);

	// Apply the y-axis rotation.
	matrix_unit(work);
	work[0x00] = cosf(input1[1]);
	work[0x02] = -sinf(input1[1]);
	work[0x08] = sinf(input1[1]);
	work[0x0A] = cosf(input1[1]);
	matrix_multiply(out_matrix, out_matrix, work);

	// Apply the z-axis rotation.
	matrix_unit(work);
	work[0x00] = cosf(input1[2]);
	work[0x01] = sinf(input1[2]);
	work[0x04] = -sinf(input1[2]);
	work[0x05] = cosf(input1[2]);
	matrix_multiply(out_matrix, out_matrix, work);

	return Matrix(out_matrix);
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
