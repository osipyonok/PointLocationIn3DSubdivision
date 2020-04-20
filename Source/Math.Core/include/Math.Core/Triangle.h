#pragma once

#include <Math.Core/API.h>

#include <Math.Core/Point3D.h>

#include <array>
#include <memory>

#include <boost/optional.hpp>

class Vector3D;

class Triangle;

MATH_CORE_API size_t hash_value(const Triangle& i_triangle);

class MATH_CORE_API Triangle
{
public:
	Triangle(const Point3D& i_point1, const Point3D& i_point2, const Point3D& i_point3);
    Triangle(const Triangle& i_other);

    Triangle(Triangle&& i_triangle);

    virtual ~Triangle();

	Point3D GetPoint(short i_index) const;

    virtual void SetPoint(short i_index, const Point3D& i_new_point);

	Vector3D GetNormal() const;

    Triangle& Flip();
	
    Triangle& operator=(const Triangle& i_other);

    bool operator==(const Triangle& i_other) const;

    size_t GetHash() const;

private:
    void _InvalidateHash();

private:
    mutable boost::optional<size_t> m_hash_cache;

	std::array<Point3D, 3> m_points;
};