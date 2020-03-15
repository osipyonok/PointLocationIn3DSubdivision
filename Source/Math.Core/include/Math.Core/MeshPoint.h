#pragma once

#include <Math.Core/API.h>

#include <Math.Core/Point3D.h>

#include <unordered_set>

class MeshTriangle;

class MATH_CORE_API MeshPoint : public Point3D
{
    Q_OBJECT

public:
    MeshPoint();
    explicit MeshPoint(const double* const ip_coordinates);
    MeshPoint(double i_x, double i_y, double i_z);

    MeshPoint(const MeshPoint& i_other) = delete;

    void AddTriangle(MeshTriangle* ip_triangle);
    void RemoveTriangle(MeshTriangle* ip_triangle);
    std::vector<MeshTriangle*> GetTriangles() const;

private:
    std::unordered_set<MeshTriangle*> m_triangles;
};
