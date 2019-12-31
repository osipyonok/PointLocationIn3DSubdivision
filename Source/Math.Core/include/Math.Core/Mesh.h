#pragma once

#include <Math.Core/API.h>

#include <memory>

class Point3D;
class Triangle;
class Vector3D;

class MATH_CORE_LIB_EXPORT Mesh final
{
public:
	Mesh();

private:
	struct Impl;
	std::unique_ptr<Impl> mp_impl;
};