#pragma once

#include <stddef.h>
#include <math.h>
#include <string.h>

#include <sstream>

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

	std::string to_string(bool print_rotation = false, bool print_w = true) const
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