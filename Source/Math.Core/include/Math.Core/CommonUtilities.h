#pragma once

#include "Math.Core/API.h"

class BoundingBox;
class Point3D;
class Triangle;

#define EPSILON (0.000000059604644775390625) // = 1/2^-24
#define PI 3.141592653589793238462643383279502884L

MATH_CORE_API double DistanceSqr(const Point3D& i_point1, const Point3D& i_point2);

MATH_CORE_API double Distance(const Point3D& i_point, const BoundingBox& i_bbox);
MATH_CORE_API double Distance(const Point3D& i_point, const Triangle& i_triangle);

MATH_CORE_API bool TriangleWithBBoxIntersection(const Triangle& i_triangle, const BoundingBox& i_bbox);

MATH_CORE_API void ExtrudeInplace(BoundingBox& i_bbox, double i_offset);
