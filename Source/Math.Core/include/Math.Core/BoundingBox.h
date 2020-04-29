#pragma once

#include <Math.Core/API.h>

#include <limits>

class Point3D;

class MATH_CORE_API BoundingBox
{
public:
    BoundingBox() = default;
    virtual ~BoundingBox() = default;

    bool IsValid() const;

    void AddPoint(const Point3D& i_point);

    bool ContainsPoint(const Point3D& i_point, const bool i_inclusive = true) const;

    Point3D GetMin() const;
    Point3D GetMax() const;

    double GetDelta(short i_index) const;
    double GetDeltaX() const;
    double GetDeltaY() const;
    double GetDeltaZ() const;

private:
    double m_max[3] = { std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest() };
    double m_min[3] = { std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max() };
};
