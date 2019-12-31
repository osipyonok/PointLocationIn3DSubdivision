#pragma once

#include <Math.Core/API.h>

#include <vector>

class Point3D;
class Triangle;

MATH_CORE_LIB_EXPORT bool ComparePointsXYZ(const Point3D& i_pt1, const Point3D& i_pt2);
MATH_CORE_LIB_EXPORT bool ComparePointsZYX(const Point3D& i_pt1, const Point3D& i_pt2);

class MATH_CORE_LIB_EXPORT Point3D
{
public:
	Point3D();
	explicit Point3D(double* i_coordinates);
	Point3D(double i_x, double i_y, double i_z);

	explicit Point3D(const Point3D& i_other);

	virtual ~Point3D();

	double  Get(short i_index) const;
	double& Get(short i_index);
	void    Set(double i_val, short i_index);

	double  GetX() const;
	double& GetX();
	double  GetY() const;
	double& GetY();
	double  GetZ() const;
	double& GetZ();

	void SetX(double i_val);
	void SetY(double i_val);
	void SetZ(double i_val);

	double  operator[](short i_index) const;
	double& operator[](short i_index);

	void AddTriangle(Triangle* ip_triangle);
	void RemoveTriangle(Triangle* ip_triangle);
	const std::vector<Triangle*>& GetTriangles() const;

	MATH_CORE_LIB_EXPORT friend bool operator<(const Point3D& i_pt1, const Point3D& i_pt2);
	MATH_CORE_LIB_EXPORT friend bool operator>(const Point3D& i_pt1, const Point3D& i_pt2);
	MATH_CORE_LIB_EXPORT friend bool operator==(const Point3D& i_pt1, const Point3D& i_pt2);

private:
	double m_coordinates[3];

	std::vector<Triangle*> m_triangles;
};