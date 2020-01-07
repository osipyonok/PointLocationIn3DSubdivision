#pragma once

#include <Math.Core/API.h>

#include <QObject>

#include <memory>
#include <vector>

class Mesh;
class Plane;
class Point3D;
class Triangle;

class MATH_CORE_API Face final : public QObject
{
	Q_OBJECT

public:
	Face(const Plane& i_plane, Mesh* ip_owner);
	Face(const Plane& i_plane, Triangle& i_root_triangle, Mesh* ip_owner);
	
	~Face() override;

	Plane GetPlane() const;

	Face* GetDualFace();

	void MergeWithFace(const Face& i_face);

	void Invalidate();

private:

	void _InvalidateDualFace();

private:
	std::unique_ptr<Plane> mp_plane;
	Triangle* mp_root_triangle = nullptr;

	std::vector<Point3D*> m_points;
	std::vector<Triangle*> m_triangles;

	Face* mp_dual_face = nullptr;
	Mesh* mp_owner = nullptr;
	bool m_dual_face_valid = false;
};
