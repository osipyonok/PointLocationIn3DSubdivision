#include "Math.Core/Triangle.h"

#include "Math.Core/Face.h"
#include "Math.Core/Plane.h"
#include "Math.Core/Vector3D.h"
#include "Math.Core/VectorUtilities.h"

#include <QtCore>

#include <algorithm>
#include <iterator>

Triangle::Triangle(Point3D* ip_point1, Point3D* ip_point2, Point3D* ip_point3, 
				   Triangle* ip_tr1, Triangle* ip_tr2, Triangle* ip_tr3, Mesh* ip_owner)
{
	m_points[0] = ip_point1;
	m_points[1] = ip_point2;
	m_points[2] = ip_point3;

	mp_face = std::make_shared<Face>(Plane(*ip_point1, GetNormal()), *this, ip_owner);

	m_neighbors[0] = ip_tr1;
	m_neighbors[1] = ip_tr2;
	m_neighbors[2] = ip_tr3;

	m_points[0]->AddTriangle(this);
	m_points[1]->AddTriangle(this);
	m_points[2]->AddTriangle(this);

	if (m_neighbors[0])
		m_neighbors[0]->_SetNeighborForEdge(m_points[1], m_points[0], this);
	if (m_neighbors[1])
		m_neighbors[1]->_SetNeighborForEdge(m_points[2], m_points[1], this);
	if (m_neighbors[2])
		m_neighbors[2]->_SetNeighborForEdge(m_points[0], m_points[2], this);

	SetOwner(ip_owner);
}

Triangle::~Triangle()
{
	m_points[0]->RemoveTriangle(this);
	m_points[1]->RemoveTriangle(this);
	m_points[2]->RemoveTriangle(this);

	if (m_neighbors[0])
		m_neighbors[0]->_RemoveNeighborForEdge(m_points[1], m_points[0], this);
	if (m_neighbors[1])
		m_neighbors[1]->_RemoveNeighborForEdge(m_points[2], m_points[1], this);
	if (m_neighbors[2])
		m_neighbors[2]->_RemoveNeighborForEdge(m_points[0], m_points[2], this);
}

Point3D* Triangle::GetPoint(short i_index) const
{
	Q_ASSERT(i_index >= 0 && i_index < 3);
	return m_points[i_index];
}

Triangle* Triangle::GetNeighbor(short i_index) const
{
	Q_ASSERT(i_index >= 0 && i_index < 3);
	return m_neighbors[i_index];
}

Vector3D Triangle::GetNormal() const
{
	return Cross({ *m_points[0], *m_points[1] }, { *m_points[0], *m_points[2] }).Normalized();
}

Mesh* Triangle::GetOwner() const
{
	return mp_owner;
}

void Triangle::SetOwner(Mesh* ip_mesh)
{
	if (ip_mesh == mp_owner)
		return;

	mp_owner = ip_mesh;

	if (m_neighbors[0])
		m_neighbors[0]->SetOwner(mp_owner);
	if (m_neighbors[1])
		m_neighbors[1]->SetOwner(mp_owner);
	if (m_neighbors[2])
		m_neighbors[2]->SetOwner(mp_owner);
}

Face* Triangle::GetFace() const
{
	return mp_face.get();
}

void Triangle::SetFace(std::shared_ptr<Face> ip_face)
{
	mp_face = ip_face;
}

void Triangle::_SetNeighborForEdge(Point3D* ip_first_point, Point3D* ip_second_point, Triangle* ip_triangle)
{
	auto it = std::find(std::begin(m_points), std::end(m_points), ip_first_point);
	Q_ASSERT(it != std::end(m_points));

	auto id = std::distance(std::begin(m_points), it);
	Q_ASSERT(m_points[(id + 2) % 3] == ip_second_point);
	Q_ASSERT(m_neighbors[(id + 2) % 3] == nullptr);

	m_neighbors[(id + 2) % 3] = ip_triangle;

	if (ip_triangle->mp_face->GetPlane() == mp_face->GetPlane())
	{
		mp_face->MergeWithFace(*ip_triangle->mp_face);
	}
}

void Triangle::_RemoveNeighborForEdge(Point3D* ip_first_point, Point3D* ip_second_point, Triangle* ip_triangle)
{
	auto it = std::find(std::begin(m_points), std::end(m_points), ip_first_point);
	Q_ASSERT(it != std::end(m_points));

	auto id = std::distance(std::begin(m_points), it);
	Q_ASSERT(m_points[(id + 2) % 3] == ip_second_point);
	Q_ASSERT(m_neighbors[(id + 2) % 3] == ip_triangle);

	m_neighbors[(id + 2) % 3] = nullptr;

	if (ip_triangle->mp_face->GetPlane() == mp_face->GetPlane())
	{
		// TODO : invalidate mp_face
	}
}

