#pragma once

#include "egg/math_types.hpp"

//#include <GL/gl.h>

// Pushes a matrix onto the matrix stack using glPushMatrix, pops the matrix
// when it goes out of scope
struct ScopedMatrix
{
	// ScopedMatrix()
	// {
	// 	glPushMatrix();
	// }

	// ScopedMatrix(const Matrix& matrix)
	// {
	// 	glPushMatrix();
	// 	glLoadMatrixf(matrix);
	// }

	// ~ScopedMatrix()
	// {
	// 	glPopMatrix();
	// }
};