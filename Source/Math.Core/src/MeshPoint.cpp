#include "Math.Core/MeshPoint.h"

#include "Math.Core/MeshTriangle.h"

#include <QtGlobal>

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

void MeshPoint::AddTriangle(MeshTriangle* ip_triangle)
{
    Q_ASSERT(m_triangles.find(ip_triangle) == m_triangles.end());
    m_triangles.insert(ip_triangle);
}

void MeshPoint::RemoveTriangle(MeshTriangle* ip_triangle)
{
    Q_ASSERT(m_triangles.find(ip_triangle) != m_triangles.end());
    m_triangles.erase(ip_triangle);
}

std::vector<MeshTriangle*> MeshPoint::GetTriangles() const
{
    return std::vector<MeshTriangle*>(m_triangles.begin(), m_triangles.end());
}
