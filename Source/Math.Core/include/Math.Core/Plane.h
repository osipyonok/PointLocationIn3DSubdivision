#pragma once

#include <Math.Core/API.h>

#include <QObject>

#include <memory>

class Point3D;
class Vector3D;


class MATH_CORE_API Plane final
{
public:
	Plane(const Point3D& i_origin, const Vector3D& i_normal);
	Plane(const Plane& i_other);

	~Plane() = default;

	const Point3D& GetOrigin() const;
	const Vector3D& GetNormal() const;

	MATH_CORE_API friend bool operator==(const Plane& i_plane1, const Plane& i_plane2);

private:
	std::unique_ptr<Point3D> mp_origin;
	std::unique_ptr<Vector3D> mp_normal;
};