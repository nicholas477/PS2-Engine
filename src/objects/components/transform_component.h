#pragma once

#include "../../types.h"


class transform_component
{
public:
	Vector location;
	Vector rotation;

	Matrix get_rotation_matrix() const;

	Vector get_forward_vector() const;
	Vector get_right_vector() const;
};