#pragma once

#include <Math.Core/API.h>

class Vector3D;


MATH_CORE_API double   Angle(const Vector3D& i_vec1, const Vector3D& i_vec2);
MATH_CORE_API Vector3D Cross(const Vector3D& i_vec1, const Vector3D& i_vec2);
MATH_CORE_API double   Dot(const Vector3D& i_vec1, const Vector3D& i_vec2);
