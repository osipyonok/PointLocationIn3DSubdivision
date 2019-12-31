#pragma once

#include <Math.Core/API.h>

#include <memory>

class Face;
class Mesh;
class Point3D;
class Vector3D;

class MATH_CORE_LIB_EXPORT Triangle final
{
public:
	Triangle(Point3D* ip_point1, Point3D* ip_point2, Point3D* ip_point3, 
			 Triangle* ip_tr1 = nullptr, Triangle* ip_tr2 = nullptr, Triangle* ip_tr3 = nullptr, Mesh* ip_owner = nullptr);
	
	~Triangle();

	Point3D* GetPoint(short i_index) const;
	
	Triangle* GetNeighbor(short i_index) const;

	Vector3D GetNormal() const;

	Mesh* GetOwner() const;
	void  SetOwner(Mesh* ip_mesh);

	Face* GetFace() const;
	void  SetFace(std::shared_ptr<Face> ip_face);

private:
	void _SetNeighborForEdge(Point3D* ip_first_point, Point3D* ip_second_point, Triangle* ip_triangle);
	void _RemoveNeighborForEdge(Point3D* ip_first_point, Point3D* ip_second_point, Triangle* ip_triangle);
	

private:
	Mesh* mp_owner = nullptr;

	std::shared_ptr<Face> mp_face;

	Point3D* m_points[3];
	Triangle* m_neighbors[3];
};