#include "egg/math_types.hpp"
#include "egg/mesh_header.hpp"
#include "egg/serialization.hpp"
#include "egg/level.hpp"
#include "egg/hashing.hpp"
#include "egg/hashmap.hpp"
#include "egg/string.hpp"

#include <sstream>

const Vector Vector::quat_identity(1.f, 0.f, 0.f, 0.f);

std::string Vector::to_string(bool print_rotation, bool print_w) const
{
	std::stringstream out_stream;
	if (print_rotation)
	{
		out_stream << "(";
		out_stream << "pitch: " << pitch;
		out_stream << ", yaw:   " << yaw;
		out_stream << ", roll:  " << roll;
		out_stream << ")";
	}
	else
	{
		out_stream << "(";
		out_stream << "x: " << x;
		out_stream << ", y: " << y;
		out_stream << ", z: " << z;
		if (print_w)
			out_stream << ", w: " << w;
		out_stream << ")";
	}
	return out_stream.str();
}

std::string Matrix::to_string() const
{
	std::stringstream out_stream;
	for (int i = 0; i < 4; ++i)
	{
		out_stream << "[";
		out_stream << "" << matrix[(i * 4) + 0];
		out_stream << ", " << matrix[(i * 4) + 1];
		out_stream << ", " << matrix[(i * 4) + 2];
		out_stream << ", " << matrix[(i * 4) + 3];
		if (i < 3)
		{
			out_stream << "]\n";
		}
		else
		{
			out_stream << "]";
		}
	}
	return out_stream.str();
}

#ifdef _EE
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


Matrix Matrix::from_location_and_rotation(const Vector& location, const Vector& rotation)
{
	return rotation.to_rotation_matrix() * location.to_translation_matrix();
}

Matrix Matrix::from_scale(const Vector& scale)
{
	Matrix output;
	output.matrix[0x00] = scale.x;
	output.matrix[0x05] = scale.y;
	output.matrix[0x0A] = scale.z;
	output.matrix[0x0F] = 1.00f;
	return output;
}
#endif