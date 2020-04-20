#pragma once

#include <Math.Core/API.h>

#include <Math.Core/Point3D.h>


class MATH_CORE_API Vector3D : public Point3D
{
public:
	Vector3D();
	explicit Vector3D(const Point3D& i_point);
	Vector3D(double i_x, double i_y, double i_z);
	Vector3D(const Point3D& i_first_point, const Point3D& i_second_point);

	Vector3D(const Vector3D& i_other) = default;
	
	~Vector3D() override = default;

	double LengthSqr() const;
	double Length() const;

	void Normalize();

	Vector3D Normalized() const;
	
	template<typename T>
	friend Vector3D& operator*(Vector3D& io_vector, const T& i_k)
	{
		io_vector.Get(0) *= i_k;
		io_vector.Get(1) *= i_k;
		io_vector.Get(2) *= i_k;
		return io_vector;
	}

	template<typename T>
	friend Vector3D& operator*(const T& i_k, Vector3D& io_vector)
	{
		return io_vector * i_k;
	}

	Vector3D& operator+=(const Vector3D& i_vector);
	Vector3D& operator-=(const Vector3D& i_vector);

	MATH_CORE_API friend Vector3D operator+(const Vector3D& i_vec1, const Vector3D& i_vec2);
	MATH_CORE_API friend Vector3D operator-(const Vector3D& i_vec1, const Vector3D& i_vec2);
};