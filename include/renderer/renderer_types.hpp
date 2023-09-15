#pragma once

#include "types.hpp"

#include <GL/gl.h>

// Pushes a matrix onto the matrix stack using glPushMatrix, pops the matrix
// when it goes out of scope
struct scoped_matrix
{
	scoped_matrix()
	{
		glPushMatrix();
	}

	scoped_matrix(const Matrix& matrix)
	{
		glPushMatrix();
		glLoadMatrixf(matrix);
	}

	~scoped_matrix()
	{
		glPopMatrix();
	}
};