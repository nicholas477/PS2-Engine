#pragma once

#include <stddef.h>
#include <math.h>

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
	};

	Vector(VECTOR _vector)
	{
		x = _vector[0];
		y = _vector[1];
		z = _vector[2];
		w = _vector[3];
	}

	Vector(float _x = 0.f, float _y = 0.f, float _z = 0.f, float _w = 0.f)
	{
		x = _x;
		y = _y;
		z = _z;
		w = _w;
	}

	float &operator[](size_t n)
	{
		return vector[n];
	}

	float length() const
	{
		return sqrt(x * x + y * y + z * z + w * w);
	}

	Vector operator+(const Vector &Rhs)
	{
		Vector out;
		vector_add(out.vector, vector, const_cast<float *>(Rhs.vector));
		return out;
	}

	Vector &operator+=(const Vector &Rhs)
	{
		vector_add(vector, vector, const_cast<float *>(Rhs.vector));
		return *this;
	}

	Vector operator*(float Rhs)
	{
		Vector out = *this;
		for (int i = 0; i < 4; ++i)
		{
			out[i] *= Rhs;
		}
		return out;
	}
} __attribute__((__aligned__(16)));