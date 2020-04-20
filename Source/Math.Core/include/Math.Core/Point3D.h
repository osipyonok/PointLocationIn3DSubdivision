#pragma once

#include <Math.Core/API.h>

#include <boost/operators.hpp>

class Point3D;

MATH_CORE_API bool ComparePointsXYZ(const Point3D& i_pt1, const Point3D& i_pt2);
MATH_CORE_API bool ComparePointsZYX(const Point3D& i_pt1, const Point3D& i_pt2);

MATH_CORE_API size_t hash_value(const Point3D& i_point);

class MATH_CORE_API Point3D 
    : private boost::addable<Point3D>
    , private boost::subtractable<Point3D>
    , private boost::less_than_comparable<Point3D>
    , private boost::equality_comparable<Point3D>
    , private boost::multipliable2<Point3D, double>
    , private boost::dividable2<Point3D, double>
{
	//Q_OBJECT

public:
	Point3D();
	explicit Point3D(const double* const ip_coordinates);
	Point3D(double i_x, double i_y, double i_z);

	Point3D(const Point3D& i_other);

    virtual ~Point3D();;// override;

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

    bool operator<(const Point3D& i_other) const;//zyx
    bool operator==(const Point3D& i_other) const;
    Point3D& operator+=(const Point3D& i_other);
    Point3D& operator-=(const Point3D& i_other);
    Point3D& operator*=(double i_value);

    Point3D& operator/=(double i_value);

    Point3D& operator=(const Point3D& i_other);

    size_t GetHash() const;

private:
	double m_coordinates[3];
};
