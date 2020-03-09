#include "Math.Core/Triangle.h"

#include "Math.Core/Vector3D.h"
#include "Math.Core/VectorUtilities.h"

#include <QtCore>

#include <algorithm>
#include <iterator>

Triangle::Triangle(Point3D* ip_point1, Point3D* ip_point2, Point3D* ip_point3, 
				   Triangle* ip_tr1, Triangle* ip_tr2, Triangle* ip_tr3)
{
	m_points[0] = ip_point1;
	m_points[1] = ip_point2;
	m_points[2] = ip_point3;

	m_neighbors[0] = ip_tr1;
	m_neighbors[1] = ip_tr2;
	m_neighbors[2] = ip_tr3;


	if (m_neighbors[0])
		m_neighbors[0]->_SetNeighborForEdge(m_points[1], m_points[0], this);
	if (m_neighbors[1])
		m_neighbors[1]->_SetNeighborForEdge(m_points[2], m_points[1], this);
	if (m_neighbors[2])
		m_neighbors[2]->_SetNeighborForEdge(m_points[0], m_points[2], this);
}

Triangle::Triangle(Triangle&& i_triangle)
{
    std::swap(m_points, i_triangle.m_points);
    std::swap(m_neighbors, i_triangle.m_neighbors);
}

Triangle::~Triangle()
{
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

void Triangle::_SetNeighborForEdge(Point3D* ip_first_point, Point3D* ip_second_point, Triangle* ip_triangle)
{
	auto it = std::find(std::begin(m_points), std::end(m_points), ip_first_point);
	Q_ASSERT(it != std::end(m_points));

	auto id = std::distance(std::begin(m_points), it);
	Q_ASSERT(m_points[(id + 2) % 3] == ip_second_point);
	Q_ASSERT(m_neighbors[(id + 2) % 3] == nullptr);

	m_neighbors[(id + 2) % 3] = ip_triangle;
}

void Triangle::_RemoveNeighborForEdge(Point3D* ip_first_point, Point3D* ip_second_point, Triangle* ip_triangle)
{
	auto it = std::find(std::begin(m_points), std::end(m_points), ip_first_point);
	Q_ASSERT(it != std::end(m_points));

	auto id = std::distance(std::begin(m_points), it);
	Q_ASSERT(m_points[(id + 2) % 3] == ip_second_point);
	Q_ASSERT(m_neighbors[(id + 2) % 3] == ip_triangle);

    m_neighbors[(id + 2) % 3] = nullptr;
}

