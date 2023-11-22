#pragma once

#include <stddef.h>
#include <math.h>
#include <string.h>
#include <string>

#ifdef _EE
#include "math3d.h"

template <typename T>
static bool is_nearly_equal(T A, T B, T ErrorTolerance)
{
	return std::abs(A - B) <= ErrorTolerance;
}

extern "C" void sincosf(float, float*, float*);
static float clamp_axis(float);
static float normalize_axis(float);
#else
#include <sstream>

typedef float VECTOR[4] __attribute__((__aligned__(16)));
typedef float MATRIX[16] __attribute__((__aligned__(16)));
#endif

struct alignas(16) Vector2
{
	union
	{
		struct
		{
			float x;
			float y;
		};
		struct
		{
			float u;
			float v;
		};
	};

	constexpr Vector2(float _x = 0.f, float _y = 0.f)
	    : x(_x)
	    , y(_y)
	{
	}
};

struct alignas(16) Vector
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

	Vector(const Vector& other)
	{
		x = other.x;
		y = other.y;
		z = other.z;
		w = other.w;
	}

	constexpr Vector(float _x = 0.f, float _y = 0.f, float _z = 0.f, float _w = 0.f)
	    : x(_x)
	    , y(_y)
	    , z(_z)
	    , w(_w)
	{
	}

	float& operator[](size_t n)
	{
		return vector[n];
	}

	Vector& operator=(const VECTOR& _vector)
	{
		memcpy(vector, _vector, sizeof(VECTOR));
		return *this;
	}

	std::string to_string(bool print_rotation = false, bool print_w = true) const;

	static const Vector quat_identity;
	static const Vector zero;

#ifdef _EE
	float length() const
	{
		return sqrt(x * x + y * y + z * z + w * w);
	}

	float distance(const Vector& other) const
	{
		return (*this - other).length();
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

	Vector& operator*=(const Vector& Rhs)
	{
		vector_multiply(vector, vector, const_cast<float*>(Rhs.vector));
		return *this;
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

	Vector& operator-=(const Vector& Rhs)
	{
		x -= Rhs.x;
		y -= Rhs.y;
		z -= Rhs.z;
		w -= Rhs.w;
		return *this;
	}

	Vector operator-() const
	{
		Vector out = *this;
		return out * -1.f;
	}

	Vector cross(const Vector& Rhs) const
	{
		Vector out;
		// out[0] = vector[1] * Rhs.vector[2] - vector[2] * Rhs.vector[1];
		// out[1] = -(vector[0] * Rhs.vector[2] - vector[2] * Rhs.vector[0]);
		// out[2] = vector[0] * Rhs.vector[1] - vector[1] * Rhs.vector[0];
		vector_cross_product(out.vector, const_cast<float*>(vector), const_cast<float*>(Rhs.vector));
		return out;
	}

	template <bool with_w = false>
	float dot(const Vector& Rhs) const
	{
		//return vector_innerproduct(const_cast<float*>(vector), const_cast<float*>(Rhs.vector)); // IDK what innerproduct does, but it definitely is NOT the dot product!

		if constexpr (with_w)
		{
			return (x * Rhs.x) + (y * Rhs.y) + (z * Rhs.z) + (w * Rhs.w);
		}
		else
		{
			return (x * Rhs.x) + (y * Rhs.y) + (z * Rhs.z);
		}
	}

	Vector normalize() const
	{
		Vector out;
		vector_normalize(out.vector, const_cast<float*>(vector));
		return out;
	}

	Vector safe_normalize(float Tolerance = 0.0001, const Vector& ResultIfZero = Vector::zero) const
	{
		const float SquareSum = x * x + y * y + z * z;

		// Not sure if it's safe to add tolerance in there. Might introduce too many errors
		if (SquareSum == 1.f)
		{
			return *this;
		}
		else if (SquareSum < Tolerance)
		{
			return ResultIfZero;
		}
		const float Scale = (float)(1.f / sqrt(SquareSum));
		return Vector(x * Scale, y * Scale, z * Scale);
	}

	// Rotation members/functions

	// clamps the axes of a rotation vector to [0, 2PI)
	Vector clamp_euler_rotation() const
	{
		Vector out = *this;
		for (int i = 0; i < 3; ++i)
		{
			out.vector[i] = clamp_axis(out.vector[i]);
		}

		return out;
	}

	// clamps the axes of a rotation vector to (-PI, PI]
	Vector normalize_euler_rotation() const
	{
		Vector out = *this;
		for (int i = 0; i < 3; ++i)
		{
			out.vector[i] = normalize_axis(out.vector[i]);
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

		return angles.clamp_euler_rotation();
	}

	// Generates a rotation matrix by applying pitch, yaw, roll (in that order)
	// to a unit matrix
	struct Matrix to_rotation_matrix() const;

	struct Matrix to_translation_matrix() const;
#endif
};

static_assert(sizeof(Vector) == sizeof(VECTOR));
static_assert(alignof(Vector) == alignof(VECTOR));
static_assert(std::is_standard_layout_v<Vector> == true);

#ifdef _EE
constexpr Vector Vector::quat_identity = Vector(1.f, 0.f, 0.f, 0.f);
constexpr Vector Vector::zero          = Vector(0.f, 0.f, 0.f, 0.f);

static Vector operator*(float Lhs, const Vector& Rhs)
{
	return Rhs * Lhs;
}
#endif

struct alignas(16) Matrix
{
	MATRIX matrix;
	Matrix(MATRIX _matrix)
	{
		memcpy(matrix, _matrix, sizeof(MATRIX));
	}

	constexpr Matrix()
	    : matrix {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f}
	{
	}

	operator const float*() const { return matrix; }
	operator float*() { return matrix; }

	float& operator[](size_t index)
	{
		return matrix[index];
	}

	const float& operator[](size_t index) const
	{
		return matrix[index];
	}

	float& lookup(size_t row, size_t col)
	{
		return matrix[col + (row * 4)];
	}

	const float& lookup(size_t row, size_t col) const
	{
		return matrix[col + (row * 4)];
	}

#ifdef _EE
	static constexpr Matrix UnitMatrix()
	{
		Matrix output;
		output.matrix[0x00] = 1.00f;
		output.matrix[0x05] = 1.00f;
		output.matrix[0x0A] = 1.00f;
		output.matrix[0x0F] = 1.00f;
		return output;
	}

	Vector transform_vector(const Vector& input) const
	{
		Vector output;
		vector_apply(output.vector, const_cast<float*>(input.vector), const_cast<float*>(matrix));
		return output;
	}

	Vector get_location() const
	{
		Vector vector;
		vector[0] = matrix[0x0C];
		vector[1] = matrix[0x0D];
		vector[2] = matrix[0x0E];
		return vector;
	}

	// Makes a matrix from a location vector and a quaternion
	static Matrix from_location_and_rotation(const Vector& location, const Vector& rotation);

	static Matrix from_scale(const Vector& scale);

	Matrix operator*(const Matrix& Rhs) const
	{
		Matrix out_matrix;
		matrix_multiply(out_matrix.matrix, const_cast<float*>(matrix), const_cast<float*>(Rhs.matrix));
		return out_matrix;
	}

	// Adds a location offset to this matrix
	Matrix operator+(const Vector& Rhs) const
	{
		Matrix out_matrix = *this;
		matrix_translate(out_matrix.matrix, const_cast<float*>(matrix), const_cast<float*>(Rhs.vector));
		return out_matrix;
	}

	Matrix invert() const
	{
		Matrix out_matrix;
		matrix_inverse(out_matrix.matrix, const_cast<float*>(matrix));
		return out_matrix;
	}

	std::string to_string() const;
#endif
};

static_assert(sizeof(Matrix) == sizeof(MATRIX));
static_assert(alignof(Matrix) == alignof(MATRIX));
static_assert(std::is_standard_layout_v<Matrix> == true);
