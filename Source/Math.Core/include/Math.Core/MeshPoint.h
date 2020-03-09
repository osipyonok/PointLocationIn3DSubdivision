#pragma once

#include <Math.Core/API.h>

#include <Math.Core/Point3D.h>

#include <unordered_set>

class Triangle;

class MATH_CORE_API MeshPoint : public Point3D
{
    Q_OBJECT

public:
    MeshPoint();
    explicit MeshPoint(const double* const ip_coordinates);
    MeshPoint(double i_x, double i_y, double i_z);

    MeshPoint(const MeshPoint& i_other) = delete;

    void AddTriangle(Triangle* ip_triangle);
    void RemoveTriangle(Triangle* ip_triangle);
    const std::vector<Triangle*>& GetTriangles() const;

private:
    std::unordered_set<Triangle*> m_triangles;
};
