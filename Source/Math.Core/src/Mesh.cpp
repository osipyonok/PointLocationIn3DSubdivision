#include "Math.Core/Mesh.h"

#include "Math.Core/MeshPoint.h"
#include "Math.Core/Triangle.h"

#include <vector>

////////////////////////////////////////////////////

struct Mesh::Impl
{
    std::vector<std::unique_ptr<MeshPoint>> m_points;
    std::vector<std::unique_ptr<Triangle>> m_triangles;
};


Mesh::Mesh()
	: mp_impl(std::make_unique<Impl>())
{
}

Mesh::~Mesh() = default;

MeshPoint* Mesh::AddPoint(const Point3D& i_point)
{
    if (auto p_point = GetPoint(i_point))
        return p_point;

    return _AddPoint(i_point.GetX(), i_point.GetY(), i_point.GetZ());
}

MeshPoint* Mesh::AddPoint(double i_x, double i_y, double i_z)
{
    if (auto p_point = GetPoint(i_x, i_y, i_z))
        return p_point;

    return _AddPoint(i_x, i_y, i_z);
}

Triangle* Mesh::AddTriangle(const Point3D& i_a, const Point3D& i_b, const Point3D& i_c)
{
    auto p_pnt1 = AddPoint(i_a);
    auto p_pnt2 = AddPoint(i_b);
    auto p_pnt3 = AddPoint(i_c);

    Q_ASSERT(p_pnt1);
    Q_ASSERT(p_pnt2);
    Q_ASSERT(p_pnt3);

    mp_impl->m_triangles.emplace_back(std::make_unique<Triangle>(p_pnt1, p_pnt2, p_pnt3));//other params
    return mp_impl->m_triangles.back().get();
}

MeshPoint* Mesh::GetPoint(const Point3D& i_point) const
{
    for (const auto& p_point : mp_impl->m_points)
    {
        if (*p_point == i_point)
            return p_point.get();
    }
    return nullptr;
}

MeshPoint* Mesh::GetPoint(double i_x, double i_y, double i_z) const
{
    return GetPoint({ i_x, i_y, i_z });
}

MeshPoint* Mesh::GetPoint(size_t i_index) const
{
    Q_ASSERT(i_index >= 0 && i_index < GetPointsCount());
    return mp_impl->m_points[i_index].get();
}

Triangle* Mesh::GetTriangleOriented(const Point3D& i_a, const Point3D& i_b, const Point3D& i_c) const
{
    auto p_pnt1 = GetPoint(i_a);
    auto p_pnt2 = GetPoint(i_b);
    auto p_pnt3 = GetPoint(i_c);

    if (!p_pnt1 || !p_pnt2 || !p_pnt3)
        return nullptr;

    if (auto p_tr = _GetTriangleSpecific(p_pnt1, p_pnt2, p_pnt3))
        return p_tr;
    if (auto p_tr = _GetTriangleSpecific(p_pnt2, p_pnt3, p_pnt1))
        return p_tr;
    if (auto p_tr = _GetTriangleSpecific(p_pnt3, p_pnt1, p_pnt2))
        return p_tr;

    return nullptr;
}

Triangle* Mesh::GetTriangle(size_t i_index) const
{
    Q_ASSERT(i_index >= 0 && i_index < GetTrianglesCount());
    return mp_impl->m_triangles[i_index].get();
}

size_t Mesh::GetPointsCount() const
{
    return mp_impl->m_points.size();
}

size_t Mesh::GetTrianglesCount() const
{
    return mp_impl->m_triangles.size();
}

MeshPoint* Mesh::_AddPoint(double i_x, double i_y, double i_z)
{
    mp_impl->m_points.emplace_back(std::make_unique<MeshPoint>(i_x, i_y, i_z));
    return mp_impl->m_points.back().get();
}

Triangle* Mesh::_GetTriangleSpecific(const Point3D* ip_a, const Point3D* ip_b, const Point3D* ip_c) const
{
    for (auto& p_tr : mp_impl->m_triangles)
    {
        if (p_tr->GetPoint(0) != ip_a)
            continue;
        if (p_tr->GetPoint(1) != ip_b)
            continue;
        if (p_tr->GetPoint(2) != ip_c)
            continue;
        return p_tr.get();
    }

    return nullptr;
}
