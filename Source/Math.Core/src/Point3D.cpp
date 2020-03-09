#include "Math.Core/Point3D.h"

#include <QtGlobal>

Point3D::Point3D()
	: Point3D(0., 0., 0.)
{
}

Point3D::Point3D(const double* const ip_coordinates)
	: Point3D(ip_coordinates[0], ip_coordinates[1], ip_coordinates[2])
{
}

Point3D::Point3D(double i_x, double i_y, double i_z)
{
	m_coordinates[0] = i_x;
	m_coordinates[1] = i_y;
	m_coordinates[2] = i_z;
}

Point3D::Point3D(const Point3D& i_other)
	: Point3D(i_other.Get(0), i_other.Get(1), i_other.Get(2))
{
}

Point3D::~Point3D() = default;

double Point3D::Get(short i_index) const
{
	Q_ASSERT(i_index >= 0 && i_index < 3);
	return m_coordinates[i_index];
}

double& Point3D::Get(short i_index)
{
	Q_ASSERT(i_index >= 0 && i_index < 3);
	return m_coordinates[i_index];
}

void Point3D::Set(double i_val, short i_index)
{
	Q_ASSERT(i_index >= 0 && i_index < 3);
	m_coordinates[i_index] = i_val;
}

double Point3D::GetX() const
{
	return Get(0);
}

double& Point3D::GetX()
{
	return Get(0);
}

double Point3D::GetY() const
{
	return Get(1);
}

double& Point3D::GetY()
{
	return Get(1);
}

double Point3D::GetZ() const
{
	return Get(2);
}

double& Point3D::GetZ()
{
	return Get(2);
}

void Point3D::SetX(double i_val)
{
	Set(i_val, 0);
}

void Point3D::SetY(double i_val)
{
	Set(i_val, 1);
}

void Point3D::SetZ(double i_val)
{
	Set(i_val, 2);
}

double Point3D::operator[](short i_index) const
{
	Q_ASSERT(i_index >= 0 && i_index < 3);
	return m_coordinates[i_index];
}

double& Point3D::operator[](short i_index)
{
	Q_ASSERT(i_index >= 0 && i_index < 3);
	return m_coordinates[i_index];
}

bool Point3D::operator<(const Point3D& i_other) const
{
    return ComparePointsZYX(*this, i_other);
}

bool Point3D::operator==(const Point3D& i_other) const
{
    return Get(0) == i_other.Get(0)
        && Get(1) == i_other.Get(1)
        && Get(2) == i_other.Get(2);
}

Point3D& Point3D::operator+=(const Point3D& i_other)
{
    Get(0) += i_other.Get(0);
    Get(1) += i_other.Get(1);
    Get(2) += i_other.Get(2);
    return *this;
}

Point3D& Point3D::operator-=(const Point3D& i_other)
{
    Get(0) -= i_other.Get(0);
    Get(1) -= i_other.Get(1);
    Get(2) -= i_other.Get(2);
    return *this;
}

Point3D& Point3D::operator*=(double i_value)
{
    Get(0) *= i_value;
    Get(1) *= i_value;
    Get(2) *= i_value;
    return *this;
}

///////////////////////////////////////////////////////////////////

bool ComparePointsXYZ(const Point3D& i_pt1, const Point3D& i_pt2)
{
	if (i_pt1.Get(0) != i_pt2.Get(0))
		return i_pt1.Get(0) < i_pt2.Get(0);
	if (i_pt1.Get(1) != i_pt2.Get(1))
		return i_pt1.Get(1) < i_pt2.Get(1);
	return i_pt1.Get(2) < i_pt2.Get(2);
}

bool ComparePointsZYX(const Point3D& i_pt1, const Point3D& i_pt2)
{
	if (i_pt1.Get(2) != i_pt2.Get(2))
		return i_pt1.Get(2) < i_pt2.Get(2);
	if (i_pt1.Get(1) != i_pt2.Get(1))
		return i_pt1.Get(1) < i_pt2.Get(1);
	return i_pt1.Get(0) < i_pt2.Get(0);
}
