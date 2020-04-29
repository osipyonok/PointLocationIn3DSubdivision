#pragma once

#include <Math.Core/API.h>

#include <Math.Core/Matrix.h>

class Point3D;
class Vector3D;


MATH_CORE_API double   Angle(const Vector3D& i_vec1, const Vector3D& i_vec2);
MATH_CORE_API Vector3D Cross(const Vector3D& i_vec1, const Vector3D& i_vec2);
MATH_CORE_API double   Dot(const Vector3D& i_vec1, const Vector3D& i_vec2);

MATH_CORE_API void Transform(Point3D& io_vector, const Matrix<double, 3>& i_mat);
