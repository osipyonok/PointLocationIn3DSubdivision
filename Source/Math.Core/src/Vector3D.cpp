#include "Math.Core/Vector3D.h"

#include "Math.Core/VectorUtilities.h"

#include <cmath>

Vector3D::Vector3D()	
	: Vector3D({0, 0, 0})
{
}

Vector3D::Vector3D(const Point3D & i_point)
	: Vector3D({0, 0, 0}, i_point)
{
}

Vector3D::Vector3D(double i_x, double i_y, double i_z)
	: Point3D(i_x, i_y, i_z)
{
}

Vector3D::Vector3D(const Point3D & i_first_point, const Point3D & i_second_point)
	: Point3D(i_second_point[0] - i_first_point[0], i_second_point[1] - i_first_point[1], i_second_point[2] - i_first_point[2])
{
}

double Vector3D::LengthSqr() const
{
	return Dot(*this, *this);
}

double Vector3D::Length() const
{
	return std::sqrt(LengthSqr());
}

void Vector3D::Normalize()
{
	const double len = Length();
	Get(0) /= len;
	Get(1) /= len;
	Get(2) /= len;
}

Vector3D Vector3D::Normalized() const
{
	Vector3D vec = *this;
	vec.Normalize();
	return vec;
}

Vector3D& Vector3D::operator+=(const Vector3D & i_vector)
{
	Get(0) += i_vector.Get(0);
	Get(1) += i_vector.Get(1);
	Get(2) += i_vector.Get(2);
	return *this;
}

Vector3D & Vector3D::operator-=(const Vector3D & i_vector)
{
	Get(0) -= i_vector.Get(0);
	Get(1) -= i_vector.Get(1);
	Get(2) -= i_vector.Get(2);
	return *this;
}

Vector3D operator+(const Vector3D & i_vec1, const Vector3D & i_vec2)
{
	Vector3D vec = i_vec1;
	vec += i_vec2;
	return vec;
}

Vector3D operator-(const Vector3D & i_vec1, const Vector3D & i_vec2)
{
	Vector3D vec = i_vec1;
	vec -= i_vec2;
	return vec;
}
