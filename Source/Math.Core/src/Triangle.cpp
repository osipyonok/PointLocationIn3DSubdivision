#include "Math.Core/Triangle.h"

#include "Math.Core/Vector3D.h"
#include "Math.Core/VectorUtilities.h"

#include <QtCore>

#include <algorithm>
#include <iterator>

#include <boost/functional/hash.hpp>

namespace
{
    inline size_t _CalculateHash(const Point3D& i_point1, const Point3D& i_point2, const Point3D& i_point3)
    {
        const std::array<const Point3D*, 3> triangle = { &i_point1, &i_point2, &i_point3 };

        short min_pos = 0;
        if (*triangle[1] < *triangle[min_pos])
            min_pos = 1;
        if (*triangle[2] < *triangle[min_pos])
            min_pos = 2;

        size_t hash = 0;
        boost::hash_combine(hash, *triangle[min_pos]);
        boost::hash_combine(hash, *triangle[(min_pos + 1) % 3]);
        boost::hash_combine(hash, *triangle[(min_pos + 2) % 3]);

        return hash;
    }
}

Triangle::Triangle(const Point3D& i_point1, const Point3D& i_point2, const Point3D& i_point3)
{
    m_points[0] = i_point1;
    m_points[1] = i_point2;
    m_points[2] = i_point3;
    _InvalidateHash();
	//if (m_neighbors[0])
	//	m_neighbors[0]->_SetNeighborForEdge(m_points[1], m_points[0], this);
	//if (m_neighbors[1])
	//	m_neighbors[1]->_SetNeighborForEdge(m_points[2], m_points[1], this);
	//if (m_neighbors[2])
	//	m_neighbors[2]->_SetNeighborForEdge(m_points[0], m_points[2], this);
}

Triangle::Triangle(const Triangle& i_other)
{
    *this = i_other;
}

Triangle::Triangle(Triangle&& i_triangle)
{
    std::swap(m_points, i_triangle.m_points);
    std::swap(m_hash_cache, i_triangle.m_hash_cache);
}

Triangle::~Triangle()
{
	//if (m_neighbors[0])
	//	m_neighbors[0]->_RemoveNeighborForEdge(m_points[1], m_points[0], this);
	//if (m_neighbors[1])
	//	m_neighbors[1]->_RemoveNeighborForEdge(m_points[2], m_points[1], this);
	//if (m_neighbors[2])
	//	m_neighbors[2]->_RemoveNeighborForEdge(m_points[0], m_points[2], this);
}

Point3D Triangle::GetPoint(short i_index) const
{
	Q_ASSERT(i_index >= 0 && i_index < 3);
	return m_points[i_index];
}

Vector3D Triangle::GetNormal() const
{
	return Cross({ m_points[0], m_points[1] }, { m_points[0], m_points[2] }).Normalized();
}

Triangle& Triangle::Flip()
{
    std::swap(m_points[0], m_points[2]);
    _InvalidateHash();
    return *this;
}

Triangle& Triangle::operator=(const Triangle& i_other)
{
    if (this != &i_other)
    {
        std::copy(std::begin(i_other.m_points), std::end(i_other.m_points), std::begin(m_points));
        m_hash_cache = i_other.m_hash_cache;
    }
    return *this;
}

bool Triangle::operator==(const Triangle& i_other) const
{
    return GetHash() == i_other.GetHash();
}

size_t Triangle::GetHash() const
{
    if (!m_hash_cache.is_initialized())
        m_hash_cache = _CalculateHash(GetPoint(0), GetPoint(1), GetPoint(2));
    return m_hash_cache.get();
}

void Triangle::_InvalidateHash()
{
    m_hash_cache.reset();
}

/*
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
*/

size_t hash_value(const Triangle& i_triangle)
{
    return i_triangle.GetHash();
}
