#pragma once

#include <Math.Core/API.h>

#include <Math.Core/Point3D.h>
#include <Math.Core/Triangle.h>

#include <list>
#include <memory>
#include <vector>

class Triangle;

class MeshTriangle;
using TriangleHandle = std::weak_ptr<MeshTriangle>;

class MATH_CORE_API MeshPoint : public Point3D
{
public:
    MeshPoint();
    explicit MeshPoint(const double* const ip_coordinates);
    MeshPoint(double i_x, double i_y, double i_z);

    MeshPoint(const MeshPoint& i_other) = delete;

    void AddTriangle(TriangleHandle ip_triangle);
    void RemoveTriangle(const Triangle& i_triangle);
    const std::list<TriangleHandle>& GetTriangles() const;
    std::vector<Point3D> GetPoints() const;
    size_t GetNeighbourPointsCount() const;

    void UpdateCoordinates(const Point3D& i_new_coordinates);

private:
    mutable std::list<TriangleHandle> m_triangles;
};
