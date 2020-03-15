#pragma once

#include <Math.Core/API.h>

#include <QObject>

#include <memory>

class MeshPoint;
class MeshTriangle;
class Point3D;
class Triangle;
class Vector3D;

class MATH_CORE_API Mesh final : public QObject
{
	Q_OBJECT

public:
	Mesh();
	~Mesh() override;

    MeshPoint* AddPoint(const Point3D& i_point);
    MeshPoint* AddPoint(double i_x, double i_y, double i_z);
    MeshTriangle* AddTriangle(const Point3D& i_a, const Point3D& i_b, const Point3D& i_c);


    MeshPoint* GetPoint(const Point3D& i_point) const;
    MeshPoint* GetPoint(double i_x, double i_y, double i_z) const;
    MeshPoint* GetPoint(size_t i_index) const;
    MeshTriangle* GetTriangleOrientationDependent(const Point3D& i_a, const Point3D& i_b, const Point3D& i_c) const;
    MeshTriangle* GetTriangleOrientationIndependent(const Point3D& i_a, const Point3D& i_b, const Point3D& i_c) const;
    MeshTriangle* GetTriangle(size_t i_index) const;

    void RemovePoint(const Point3D& i_point);
    void RemovePoint(double i_x, double i_y, double i_z);
    void RemoveTriangle(const Point3D& i_a, const Point3D& i_b, const Point3D& i_c);

    size_t GetPointsCount() const;
    size_t GetTrianglesCount() const;

private:
	struct Impl;
	std::unique_ptr<Impl> mp_impl;
};