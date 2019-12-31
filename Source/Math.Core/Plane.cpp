#include "Math.Core/Plane.h"

#include "Math.Core/CommonUtilities.h"

#include "Math.Core/Point3D.h"
#include "Math.Core/Vector3D.h"
#include "Math.Core/VectorUtilities.h"


namespace 
{
	constexpr double _angle_eps = 0.0005;
}


Plane::Plane(const Point3D& i_origin, const Vector3D& i_normal)
	: mp_origin(std::make_unique<Point3D>(i_origin))
	, mp_normal(std::make_unique<Vector3D>(i_normal.Normalized()))
{
}

Plane::Plane(const Plane& i_other)
	: Plane(i_other.GetOrigin(), i_other.GetNormal())
{
}

const Point3D& Plane::GetOrigin() const
{
	return *mp_origin;
}

const Vector3D& Plane::GetNormal() const
{
	return *mp_normal;
}

bool operator==(const Plane& i_plane1, const Plane& i_plane2)
{
	const auto angle = Angle(i_plane1.GetNormal(), i_plane2.GetNormal());

	if (std::abs(angle) > _angle_eps)
		return false;

	return std::abs(Dot(i_plane1.GetNormal(), Vector3D(i_plane2.GetOrigin()) - Vector3D(i_plane1.GetOrigin()))) < EPSILON;
}
