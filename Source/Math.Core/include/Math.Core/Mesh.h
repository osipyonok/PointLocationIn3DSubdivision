#pragma once

#include <Math.Core/API.h>

#include <QObject>

#include <memory>

class MeshPoint;
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
    Triangle* AddTriangle(const Point3D& i_a, const Point3D& i_b, const Point3D& i_c);


    MeshPoint* GetPoint(const Point3D& i_point) const;
    MeshPoint* GetPoint(double i_x, double i_y, double i_z) const;
    MeshPoint* GetPoint(size_t i_index) const;
    Triangle* GetTriangleOriented(const Point3D& i_a, const Point3D& i_b, const Point3D& i_c) const;
    Triangle* GetTriangle(size_t i_index) const;

    size_t GetPointsCount() const;
    size_t GetTrianglesCount() const;

private:
    MeshPoint* _AddPoint(double i_x, double i_y, double i_z);

    Triangle* _GetTriangleSpecific(const Point3D* ip_a, const Point3D* ip_b, const Point3D* ip_c) const;
private:
	struct Impl;
	std::unique_ptr<Impl> mp_impl;
};