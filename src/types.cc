#include "types.hpp"

#include <sstream>

Matrix Vector::to_rotation_matrix() const
{
	MATRIX out_matrix;
	matrix_unit(out_matrix);

	// This function is reversed from the normal matrix rotation
	// implementation. This applies pitch, yaw, roll, instead of
	// applying roll, yaw, pitch

	// Apply the x-axis rotation.
	MATRIX work;
	matrix_unit(work);
	work[0x05] = cosf(vector[0]);
	work[0x06] = sinf(vector[0]);
	work[0x09] = -sinf(vector[0]);
	work[0x0A] = cosf(vector[0]);
	matrix_multiply(out_matrix, out_matrix, work);

	// Apply the y-axis rotation.
	matrix_unit(work);
	work[0x00] = cosf(vector[1]);
	work[0x02] = -sinf(vector[1]);
	work[0x08] = sinf(vector[1]);
	work[0x0A] = cosf(vector[1]);
	matrix_multiply(out_matrix, out_matrix, work);

	// Apply the z-axis rotation.
	matrix_unit(work);
	work[0x00] = cosf(vector[2]);
	work[0x01] = sinf(vector[2]);
	work[0x04] = -sinf(vector[2]);
	work[0x05] = cosf(vector[2]);
	matrix_multiply(out_matrix, out_matrix, work);

	return Matrix(out_matrix);
}

Matrix Vector::to_translation_matrix() const
{
	MATRIX out_matrix;
	matrix_unit(out_matrix);
	out_matrix[0x0C] = vector[0];
	out_matrix[0x0D] = vector[1];
	out_matrix[0x0E] = vector[2];

	return Matrix(out_matrix);
}

std::string Vector::to_string(bool print_rotation, bool print_w) const
{
	std::stringstream out_stream;
	if (print_rotation)
	{
		out_stream << "pitch: " << pitch << "\n";
		out_stream << "yaw:   " << yaw << "\n";
		out_stream << "roll:  " << roll << "\n";
	}
	else
	{
		out_stream << "x: " << x << "\n";
		out_stream << "y: " << y << "\n";
		out_stream << "z: " << z << "\n";
		if (print_w)
			out_stream << "w: " << w << "\n";
	}
	return out_stream.str();
}

Matrix Matrix::from_location_and_rotation(const Vector& location, const Vector& rotation)
{
	return rotation.to_rotation_matrix() * location.to_translation_matrix();
}