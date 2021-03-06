#pragma once

#include <Math.Core/API.h>

#include <QObject>

#include <memory>
#include <vector>

class BoundingBox;
class MeshPoint;
class MeshTriangle;
class Point3D;
class Triangle;
class Vector3D;

using TriangleHandle = std::weak_ptr<MeshTriangle>;

class QString;

class MATH_CORE_API Mesh final : public QObject
{
	Q_OBJECT
    Q_DISABLE_COPY(Mesh)

public:
	Mesh();
    Mesh(Mesh&& i_mesh);

	~Mesh() override;

    MeshPoint* AddPoint(const Point3D& i_point);
    MeshPoint* AddPoint(double i_x, double i_y, double i_z);
    TriangleHandle AddTriangle(const Point3D& i_a, const Point3D& i_b, const Point3D& i_c);


    MeshPoint* GetPoint(const Point3D& i_point) const;
    MeshPoint* GetPoint(double i_x, double i_y, double i_z) const;
    MeshPoint* GetPoint(size_t i_index) const;
    TriangleHandle GetTriangleOrientationDependent(const Point3D& i_a, const Point3D& i_b, const Point3D& i_c) const;
    TriangleHandle GetTriangleOrientationIndependent(const Point3D& i_a, const Point3D& i_b, const Point3D& i_c) const;
    TriangleHandle GetTriangle(size_t i_index) const;
    TriangleHandle GetTriangle(const Triangle& i_triangle) const;

    std::vector<TriangleHandle> GetTrianglesIncidentToEdge(const Point3D& i_a, const Point3D& i_b) const;
    std::vector<TriangleHandle> GetTrianglesIncidentToPoint(const Point3D& i_point) const;

    void UpdatePointCoordinates(const Point3D& i_old_coordinates, const Point3D& i_new_coordinates);

    void RemovePoint(const Point3D& i_point);
    void RemovePoint(double i_x, double i_y, double i_z);
    void RemoveTriangle(const Point3D& i_a, const Point3D& i_b, const Point3D& i_c);

    size_t GetPointsCount() const;
    size_t GetTrianglesCount() const;

    const QString& GetName() const;
    void SetName(const QString& i_name) const;

    const BoundingBox& GetBoundingBox() const;

private:
    void _InvalidateCache();

private:
	struct Impl;
	std::unique_ptr<Impl> mp_impl;
};