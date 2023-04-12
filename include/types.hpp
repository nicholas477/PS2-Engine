#pragma once

#include <stddef.h>
#include <math.h>
#include <string.h>
#include <string>

#include "math3d.h"

extern "C" void sincosf(float, float*, float*);

struct Vector
{
	union
	{
		VECTOR vector;
		struct
		{
			float x;
			float y;
			float z;
			float w;
		};
		struct
		{
			float pitch;
			float yaw;
			float roll;
		};
	};

	Vector(VECTOR _vector)
	{
		memcpy(vector, _vector, sizeof(VECTOR));
	}

	Vector(float _x = 0.f, float _y = 0.f, float _z = 0.f, float _w = 0.f)
	{
		x = _x;
		y = _y;
		z = _z;
		w = _w;
	}

	float& operator[](size_t n)
	{
		return vector[n];
	}

	float length() const
	{
		return sqrt(x * x + y * y + z * z + w * w);
	}

	Vector operator+(const Vector& Rhs) const
	{
		Vector out;
		vector_add(out.vector, const_cast<float*>(vector), const_cast<float*>(Rhs.vector));
		return out;
	}

	Vector& operator+=(const Vector& Rhs)
	{
		vector_add(vector, vector, const_cast<float*>(Rhs.vector));
		return *this;
	}

	Vector operator*(float Rhs) const
	{
		Vector out = *this;
		for (int i = 0; i < 4; ++i)
		{
			out[i] *= Rhs;
		}
		return out;
	}

	Vector operator/(float Rhs) const
	{
		Vector out = *this;
		for (int i = 0; i < 4; ++i)
		{
			out[i] /= Rhs;
		}
		return out;
	}

	Vector& operator/=(float Rhs)
	{
		*this = *this / Rhs;
		return *this;
	}

	Vector operator-(const Vector& Rhs) const
	{
		Vector out = *this;
		out.x -= Rhs.x;
		out.y -= Rhs.y;
		out.z -= Rhs.z;
		out.w -= Rhs.w;
		return out;
	}

	Vector operator-() const
	{
		Vector out = *this;
		return out * -1.f;
	}

	Vector cross(const Vector& Rhs) const
	{
		Vector out;
		vector_cross_product(out.vector, const_cast<float*>(vector), const_cast<float*>(Rhs.vector));
		return out;
	}

	float dot(const Vector& Rhs) const
	{
		//return vector_innerproduct(const_cast<float*>(vector), const_cast<float*>(Rhs.vector));
		return (x * Rhs.x) + (y * Rhs.y) + (z * Rhs.z) + (w * Rhs.w);
	}

	Vector normalize() const
	{
		Vector out;
		vector_normalize(out.vector, const_cast<float*>(vector));
		return out;
	}

	// Rotation members/functions

	// Normalizes the axes of a rotation vector to [0, 2PI)
	Vector normalize_euler_rotation() const
	{
		Vector out = *this;
		for (int i = 0; i < 3; ++i)
		{
			out.vector[i] = fmod(vector[i], 2 * M_PI);
			if (out.vector[i] < 0.0)
			{
				out.vector[i] += 2 * M_PI;
			}

			// if (out.vector[i] > M_PI)
			// {
			// 	out.vector[i] -= 2 * M_PI;
			// }
		}

		return out;
	}

	Vector normalize_quat(float Tolerance = 1.e-8f) const
	{
		Vector out            = *this;
		const float SquareSum = x * x + y * y + z * z + w * w;

		if (SquareSum >= Tolerance)
		{
			const float Scale = 1.f / sqrt(SquareSum);

			out.x *= Scale;
			out.y *= Scale;
			out.z *= Scale;
			out.w *= Scale;

			return out;
		}
		else
		{
			return quat_identity;
		}
	}

	Vector euler_to_quat() const
	{
		double cr = cos(roll * 0.5);
		double sr = sin(roll * 0.5);
		double cp = cos(pitch * 0.5);
		double sp = sin(pitch * 0.5);
		double cy = cos(yaw * 0.5);
		double sy = sin(yaw * 0.5);


		Vector q;
		q.w = cr * cp * cy + sr * sp * sy;
		q.x = sr * cp * cy - cr * sp * sy;
		q.y = cr * sp * cy + sr * cp * sy;
		q.z = cr * cp * sy - sr * sp * cy;

		return q;
	}

	Vector quat_to_euler() const
	{
		Vector angles;

		// roll (x-axis rotation)
		double sinr_cosp = 2 * (w * x + y * z);
		double cosr_cosp = 1 - 2 * (x * x + y * y);
		angles.roll      = std::atan2(sinr_cosp, cosr_cosp);

		// pitch (y-axis rotation)
		double sinp  = std::sqrt(1 + 2 * (w * y - x * z));
		double cosp  = std::sqrt(1 - 2 * (w * y - x * z));
		angles.pitch = 2 * std::atan2(sinp, cosp) - M_PI / 2;

		// yaw (z-axis rotation)
		double siny_cosp = 2 * (w * z + x * y);
		double cosy_cosp = 1 - 2 * (y * y + z * z);
		angles.yaw       = std::atan2(siny_cosp, cosy_cosp);

		return angles.normalize_euler_rotation();
	}

	// Generates a rotation matrix by applying pitch, yaw, roll (in that order)
	// to a unit matrix
	struct Matrix to_rotation_matrix() const;

	struct Matrix to_translation_matrix() const;

	std::string to_string(bool print_rotation = false, bool print_w = true) const;

	static const Vector quat_identity;
} __attribute__((__aligned__(16)));

static Vector operator*(float Lhs, const Vector& Rhs)
{
	return Rhs * Lhs;
}

struct Matrix
{
	MATRIX matrix;
	Matrix(MATRIX _matrix)
	{
		memcpy(matrix, _matrix, sizeof(MATRIX));
	}

	Matrix() = default;

	Vector transform_vector(const Vector& input) const
	{
		Vector output;
		vector_apply(output.vector, const_cast<float*>(input.vector), const_cast<float*>(matrix));
		return output;
	}

	static Matrix from_location_and_rotation(const Vector& location, const Vector& rotation);

	Matrix operator*(const Matrix& Rhs) const
	{
		Matrix out_matrix;
		matrix_multiply(out_matrix.matrix, const_cast<float*>(matrix), const_cast<float*>(Rhs.matrix));
		return out_matrix;
	}

	Matrix invert() const
	{
		Matrix out_matrix;
		matrix_inverse(out_matrix.matrix, const_cast<float*>(matrix));
		return out_matrix;
	}
} __attribute__((__aligned__(16)));