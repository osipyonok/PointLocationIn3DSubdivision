#include "Math.Core/MeshPoint.h"

#include "Math.Core/MeshTriangle.h"

#include <QtGlobal>

#include <cassert>
#include <set>


MeshPoint::MeshPoint()
{
}

MeshPoint::MeshPoint(const double* const ip_coordinates)
    : Point3D(ip_coordinates)
{
}

MeshPoint::MeshPoint(double i_x, double i_y, double i_z)
    : Point3D(i_x, i_y, i_z)
{
}

void MeshPoint::AddTriangle(TriangleHandle ip_triangle)
{
    if (auto p_tr = ip_triangle.lock())
    {
        m_triangles.push_back(ip_triangle);
    }
    else
    {
        assert(false);
    }
}

void MeshPoint::RemoveTriangle(const Triangle& i_triangle)
{

    auto it = std::find_if(m_triangles.begin(), m_triangles.begin(), [&](const TriangleHandle& i_tr_handle)
    {
        if (auto p_tr = i_tr_handle.lock())
            return *p_tr == i_triangle;
        return false;
    });

    if(it != m_triangles.end())
        m_triangles.erase(it);
}

const std::list<TriangleHandle>& MeshPoint::GetTriangles() const
{
    for (auto it = m_triangles.begin(); it != m_triangles.end();)
    {
        if (!it->lock())
        {
            it = m_triangles.erase(it);
        }
        else
        {
            ++it;
        }
    }
    return m_triangles;
}

std::vector<Point3D> MeshPoint::GetPoints() const
{
    std::set<Point3D> points;
    for (auto it = m_triangles.begin(); it != m_triangles.end();)
    {
        if (auto p_shared_triangle = it->lock())
        {
            if (p_shared_triangle->GetPoint(0) != *this)
                points.emplace(p_shared_triangle->GetPoint(0));
            if (p_shared_triangle->GetPoint(1) != *this)
                points.emplace(p_shared_triangle->GetPoint(1));
            if (p_shared_triangle->GetPoint(2) != *this)
                points.emplace(p_shared_triangle->GetPoint(2));
            ++it;
        }
        else
        {
            it = m_triangles.erase(it);
        }
    }
    
    return std::vector<Point3D>{ points.begin(), points.end() };
}

size_t MeshPoint::GetNeighbourPointsCount() const
{
    return GetPoints().size();
}

void MeshPoint::UpdateCoordinates(const Point3D& i_new_coordinates)
{
    Point3D old_coords = *this;
    if (old_coords == i_new_coordinates)
        return;

    SetX(i_new_coordinates.GetX());
    SetY(i_new_coordinates.GetY());
    SetZ(i_new_coordinates.GetZ());
}
