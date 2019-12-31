#include "Math.Core/Face.h"

#include "Math.Core/Plane.h"
#include "Math.Core/Triangle.h"
#include "Math.Core/Vector3D.h"

Face::Face(const Plane& i_plane, Mesh* ip_owner)
	: mp_plane(std::make_unique<Plane>(i_plane))
	, mp_owner(ip_owner)
{
}

Face::Face(const Plane& i_plane, Triangle& i_root_triangle, Mesh* ip_owner)
	: Face(i_plane, ip_owner)
{
	mp_root_triangle = &i_root_triangle;
}

Face::~Face()
{
	if (auto p_dual_face = GetDualFace())
	{
		p_dual_face->mp_dual_face = nullptr;
		p_dual_face->m_dual_face_valid = false; // ?
	}
}

Plane Face::GetPlane() const
{
	return *mp_plane;
}

Face* Face::GetDualFace()
{
	if (m_dual_face_valid)
		return mp_dual_face;




	m_dual_face_valid = true;
}

void Face::MergeWithFace(const Face& i_face)
{
	Q_ASSERT(mp_owner == i_face.mp_owner);
	Q_ASSERT(GetPlane() == i_face.GetPlane());

	m_points.reserve(m_points.size() + i_face.m_points.size());
	std::copy(i_face.m_points.begin(), i_face.m_points.end(), std::back_inserter(m_points));
	m_points.erase(std::unique(m_points.begin(), m_points.end()), m_points.end());

	m_triangles.reserve(m_triangles.size() + i_face.m_triangles.size());
	std::copy(i_face.m_triangles.begin(), i_face.m_triangles.end(), std::back_inserter(m_triangles));

	for (auto& p_triangle : i_face.m_triangles)
	{
	//	p_triangle->SetFace()
	}

	_InvalidateDualFace();
}

void Face::Invalidate()
{

}

void Face::_InvalidateDualFace()
{
	if (auto p_dual_face = GetDualFace())
	{
		p_dual_face->mp_dual_face = nullptr;
		p_dual_face->m_dual_face_valid = false;
	}
	
	mp_dual_face = nullptr;
	m_dual_face_valid = false;
}
