#pragma once

#include <stddef.h>
#include <math.h>
#include <string.h>
#include <string>

#include "math3d.h"

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

	Vector normalize() const
	{
		Vector out;
		vector_normalize(out.vector, const_cast<float*>(vector));
		return out;
	}

	// Rotation members/functions

	// Normalizes the axes of a rotation vector to [0, 2PI)
	Vector normalize_rotation() const
	{
		Vector out = *this;
		for (int i = 0; i < 3; ++i)
		{
			out.vector[i] += 2 * M_PI;
			out.vector[i] = fmod(out.vector[i], 2 * M_PI);
		}

		return out;
	}

	// Generates a rotation matrix by applying pitch, yaw, roll (in that order)
	// to a unit matrix
	struct Matrix to_rotation_matrix() const;

	std::string to_string(bool print_rotation = false, bool print_w = true) const;
} __attribute__((__aligned__(16)));

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
} __attribute__((__aligned__(16)));