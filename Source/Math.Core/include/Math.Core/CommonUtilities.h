#pragma once

#include "Math.Core/API.h"

class BoundingBox;
class Point3D;
class Triangle;

#define EPSILON (0.000000059604644775390625) // = 1/2^-24

MATH_CORE_API double Distance(const Point3D& i_point, const BoundingBox& i_bbox);
MATH_CORE_API double Distance(const Point3D& i_point, const Triangle& i_triangle);
