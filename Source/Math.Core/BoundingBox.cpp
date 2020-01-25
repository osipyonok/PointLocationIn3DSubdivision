#include "Math.Core/BoundingBox.h"

#include "Math.Core/Point3D.h"

#include <functional>
#include <utility>

namespace
{

}

bool BoundingBox::IsValid() const
{
    return m_max[0] >= m_min[0]
        && m_max[1] >= m_min[1]
        && m_max[2] >= m_min[2];
}

void BoundingBox::AddPoint(const Point3D& i_point)
{
    auto x = i_point.Get(0);
    auto y = i_point.Get(1);
    auto z = i_point.Get(2);

    if (m_min[0] > x)
        m_min[0] = x;
    if (m_min[1] > y)
        m_min[1] = y;
    if (m_min[2] > z)
        m_min[2] = z;

    if (m_max[0] < x)
        m_max[0] = x;
    if (m_max[1] < y)
        m_max[1] = y;
    if (m_max[2] < z)
        m_max[2] = z;
}

bool BoundingBox::ContainsPoint(const Point3D& i_point, const bool i_inclusive) const
{
    auto x = i_point.Get(0);
    auto y = i_point.Get(1);
    auto z = i_point.Get(2);
    
    auto less_comparison = i_inclusive ? std::function<bool(double, double)>([](auto i_lhs, auto i_rhs) { return i_lhs <  i_rhs; })
                                       : std::function<bool(double, double)>([](auto i_lhs, auto i_rhs) { return i_lhs <= i_rhs; });

    if (less_comparison(x, m_min[0]))
        return false;
    if (less_comparison(y, m_min[1]))
        return false;
    if (less_comparison(z, m_min[2]))
        return false;

    if (less_comparison(m_max[0], x))
        return false;
    if (less_comparison(m_max[1], y))
        return false;
    if (less_comparison(m_max[2], z))
        return false;

    return true;
}

Point3D BoundingBox::GetMin() const
{
    return Point3D(m_min);
}

Point3D BoundingBox::GetMax() const
{
    return Point3D(m_max);
}

double BoundingBox::GetDelta(short i_index) const
{
    return m_max[i_index] - m_min[i_index];
}

double BoundingBox::GetDeltaX() const
{
    return GetDelta(0);
}

double BoundingBox::GetDeltaY() const
{
    return GetDelta(1);
}

double BoundingBox::GetDeltaZ() const
{
    return GetDelta(2);
}
