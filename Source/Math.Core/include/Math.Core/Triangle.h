#pragma once

#include <Math.Core/API.h>

#include <QObject>

#include <memory>

class Point3D;
class Vector3D;

class MATH_CORE_API Triangle final : public QObject
{
	Q_OBJECT

public:
	Triangle(Point3D* ip_point1, Point3D* ip_point2, Point3D* ip_point3, 
			 Triangle* ip_tr1 = nullptr, Triangle* ip_tr2 = nullptr, Triangle* ip_tr3 = nullptr);
	
    Triangle(Triangle&& i_triangle);

	~Triangle() override;

	Point3D* GetPoint(short i_index) const;
	
	Triangle* GetNeighbor(short i_index) const;

	Vector3D GetNormal() const;

private:
	void _SetNeighborForEdge(Point3D* ip_first_point, Point3D* ip_second_point, Triangle* ip_triangle);
	void _RemoveNeighborForEdge(Point3D* ip_first_point, Point3D* ip_second_point, Triangle* ip_triangle);
	

private:

	Point3D* m_points[3];
	Triangle* m_neighbors[3];
};