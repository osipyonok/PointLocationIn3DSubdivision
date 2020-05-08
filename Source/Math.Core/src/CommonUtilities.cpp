#include "Math.Core/CommonUtilities.h"

#include "Math.Core/BoundingBox.h"
#include "Math.Core/Plane.h"
#include "Math.Core/Point3D.h"
#include "Math.Core/Triangle.h"
#include "Math.Core/Vector3D.h"
#include "Math.Core/VectorUtilities.h"

#include <cmath>
#include <tuple>
#include <utility>
#include <vector>


namespace
{
    inline void _UpdateMinMax(double i_x0, double i_x1, double i_x2, double& o_min, double& o_max)
    {
        o_min = o_max = i_x0;
        if (o_min > i_x1)
            o_min = i_x1;
        if (o_min > i_x2)
            o_min = i_x2;
        if (o_max < i_x1)
            o_max = i_x1;
        if (o_max < i_x2)
            o_max = i_x2;
    }

    inline bool _AxisTestX(double i_a, double i_b, const Point3D& i_p0, const Point3D& i_p1, const Point3D& i_box_halfdelta)
    {
        auto abs_a = i_a < 0 ? -i_a : i_a;
        auto abs_b = i_b < 0 ? -i_b : i_b;

        auto p1 = i_a * i_p0.GetY() - i_b * i_p0.GetZ();
        auto p2 = i_a * i_p1.GetY() - i_b * i_p1.GetZ();

        auto min = p1 < p2 ? p1 : p2;
        auto max = p1 < p2 ? p2 : p1;

        auto rad = abs_a * i_box_halfdelta.GetY() + abs_b * i_box_halfdelta.GetZ();

        if (min > rad + DBL_EPSILON || max + DBL_EPSILON < -rad)
            return false;
        return true;
    }

    inline bool _AxisTestY(double i_a, double i_b, const Point3D& i_p0, const Point3D& i_p1, const Point3D& i_box_halfdelta)
    {
        auto abs_a = i_a < 0 ? -i_a : i_a;
        auto abs_b = i_b < 0 ? -i_b : i_b;

        auto p1 = -i_a * i_p0.GetX() + i_b * i_p0.GetZ();
        auto p2 = -i_a * i_p1.GetX() + i_b * i_p1.GetZ();

        auto min = p1 < p2 ? p1 : p2;
        auto max = p1 < p2 ? p2 : p1;

        auto rad = abs_a * i_box_halfdelta.GetX() + abs_b * i_box_halfdelta.GetZ();

        if (min > rad + DBL_EPSILON || max + DBL_EPSILON < -rad)
            return false;
        return true;
    }

    inline bool _AxisTestZ(double i_a, double i_b, const Point3D& i_p0, const Point3D& i_p1, const Point3D& i_box_halfdelta)
    {
        auto abs_a = i_a < 0 ? -i_a : i_a;
        auto abs_b = i_b < 0 ? -i_b : i_b;

        auto p1 = i_a * i_p0.GetX() - i_b * i_p0.GetY();
        auto p2 = i_a * i_p1.GetX() - i_b * i_p1.GetY();

        auto min = p1 < p2 ? p1 : p2;
        auto max = p1 < p2 ? p2 : p1;

        auto rad = abs_a * i_box_halfdelta.GetX() + abs_b * i_box_halfdelta.GetY();

        if (min > rad + DBL_EPSILON || max + DBL_EPSILON < -rad)
            return false;
        return true;
    }

    inline bool _PlaneBoxOverlap(const Point3D& i_point, const Vector3D& i_normal, const Point3D& i_box_halfdelta)
    {
        Vector3D min, max;
        for (size_t i = 0; i < 3; ++i)
        {
            if (i_normal.Get(static_cast<short>(i)) > 0)
            {
                min.Set(-i_box_halfdelta.Get(static_cast<short>(i)) - i_point.Get(static_cast<short>(i)), static_cast<short>(i));
                max.Set( i_box_halfdelta.Get(static_cast<short>(i)) - i_point.Get(static_cast<short>(i)), static_cast<short>(i));
            }
            else
            {
                min.Set( i_box_halfdelta.Get(static_cast<short>(i)) - i_point.Get(static_cast<short>(i)), static_cast<short>(i));
                max.Set(-i_box_halfdelta.Get(static_cast<short>(i)) - i_point.Get(static_cast<short>(i)), static_cast<short>(i));
            }
        }

        if (Dot(i_normal, min) > DBL_EPSILON)
            return false;
        if (Dot(i_normal, max) >= -DBL_EPSILON)
            return true;
        return false;
    }
}


double DistanceSqr(const Point3D& i_point1, const Point3D& i_point2)
{
    const auto tmp = i_point1 - i_point2;
    return tmp.Get(0) * tmp.Get(0) + tmp.Get(1) * tmp.Get(1) + tmp.Get(2) * tmp.Get(2);
}

double Distance(const Point3D& i_point, const BoundingBox& i_bbox)
{
    if (i_bbox.ContainsPoint(i_point))
        return 0;

    std::vector<Point3D> nodes;
    
    auto min_point = i_bbox.GetMin();
    auto max_point = i_bbox.GetMax();

    for (auto i = 0u; i < 8; ++i)
    {
        nodes.emplace_back(
            (i & 1) ? max_point.Get(0) : min_point.Get(0),
            (i & 2) ? max_point.Get(1) : min_point.Get(1),
            (i & 4) ? max_point.Get(2) : min_point.Get(2)
        );
    }

    static std::vector<std::tuple<int,int,int>> triangles = // xyz ordered indexes 
    {
        // bottom face
        {0, 1, 2},
        {1, 2, 3},
        // front face
        {0, 1, 5},
        {0, 5, 4},
        // left face
        {0, 2, 6},
        {0, 6, 4},
        // right face
        {1, 3, 7},
        {1, 7, 5},
        // back face
        {2, 3, 7},
        {2, 7, 6},
        // top face
        {4, 5, 7},
        {4, 7, 6}
    };

    auto distance = std::numeric_limits<double>::max();
    for (auto indexes : triangles)
    {
        Triangle triangle(nodes[std::get<0>(indexes)], nodes[std::get<1>(indexes)], nodes[std::get<2>(indexes)]);
        distance = std::min(distance, Distance(i_point, triangle));
    }

    return distance;
}

double Distance(const Point3D& i_point, const Triangle& i_triangle)
{
    auto diff = i_point - i_triangle.GetPoint(0);
    auto edge1 = i_triangle.GetPoint(1) - i_triangle.GetPoint(0);
    auto edge2 = i_triangle.GetPoint(2) - i_triangle.GetPoint(0);

    auto a00 = Vector3D(edge1).LengthSqr();
    auto a01 = Dot(Vector3D(edge1), Vector3D(edge2));
    auto a11 = Vector3D(edge2).LengthSqr();

    auto b0 = -Dot(Vector3D(diff), Vector3D(edge1));
    auto b1 = -Dot(Vector3D(diff), Vector3D(edge2));

    auto f00 = b0;
    auto f10 = b0 + a00;
    auto f01 = b0 + a01;

    using _Point2D = std::pair<double, double>;

    auto get_min_edge02 = [=]() -> _Point2D
    {
        return { 0 , b1 >= 0 ? 0 : (a11 + b1 <= 0 ? 1 : -b1 / a11) };
    };

    auto get_min_edge12 = [=]() -> _Point2D
    {
        auto h0 = a01 + b1 - f10;
        auto h1 = a11 + b1 - f01;
        auto y = h0 < 0 ? (h1 <= 0 ? 1. : (h0 / (h0 - h1))) : 0;
        return { 1 - y, y };
    };

    auto get_min_inside = [](const auto& i_p0, const auto& i_p1, double h0, double h1) -> _Point2D
    {
        auto z = h0 / (h0 - h1);
        return { (1 - z) * i_p0.first + z * i_p1.first, (1 - z) * i_p0.second + z * i_p1.second };
    };

    _Point2D point = { 0, 0 };

    if (f00 >= 0)
    {
        if (f01 < 0)
        {
            _Point2D point1 = { 0., f00 / (f00 - f01) };
            _Point2D point2 = { f01 / (f01 - f10), 1. - f01 / (f01 - f10) };

            auto h0 = (point2.second - point1.second) * (a11 * point1.second + b1);
            if (h0 >= 0)
            {
                point = get_min_edge02();
            }
            else
            {
                auto h1 = (point2.second - point1.second) * (a01 * point1.first + a11 * point1.second + b1);
                point = h1 <= 0 ? get_min_edge12() : get_min_inside(point1, point2, h0, h1);
            }
        }
        else
        {
            point = get_min_edge02();
        }
    }
    else if (f01 <= 0)
    {
        if (f10 <= 0)
        {
            point = get_min_edge12();
        }
        else
        {
            _Point2D point1 = { f00 / (f00 - f10), 0. };
            _Point2D point2 = { f01 / (f01 - f10), 1. - f01 / (f01 - f10) };
            auto h0 = point2.second * (a01 * point1.first + b1);
            if (h0 >= 0)
            {
                point = point1;
            }
            else
            {
                auto h1 = point2.second * (a01 * point2.first + a11 * point2.second + b1);
                point = h1 <= 0 ? get_min_edge12() : get_min_inside(point1, point2, h0, h1);
            }
        }
    }
    else if (f10 <= 0)
    {
        _Point2D point1 = { 0, f00 / (f00 - f01) };
        _Point2D point2 = { f01 / (f01 - f10), 1. - f01 / (f01 - f10) };
        auto dt1 = point2.second - point1.second;
        auto h0 = dt1 * (a11 * point1.second + b1);
        if (h0 >= 0)
        {
            point = get_min_edge02();
        }
        else
        {
            auto h1 = dt1 * (a01 * point2.first + a11 * point2.second + b1);
            point = h1 <= 0 ? get_min_edge12() : get_min_inside(point1, point2, h0, h1);
        }
    }
    else
    {
        _Point2D point1 = { f00 / (f00 - f10), 0 };
        _Point2D point2 = { 0, f00 / (f00 - f01) };
        auto h0 = point2.second * (a01 * point1.first + b1);
        if (h0 >= 0)
        {
            point = point1;
        }
        else
        {
            auto h1 = point2.second * (a11 * point2.second + b1);
            point = h1 <= 0 ? get_min_edge02() : get_min_inside(point1, point2, h0, h1);
        }
    }

    auto nearest = i_triangle.GetPoint(0) + point.first * edge1 + point.second * edge2;
    auto dist_sqr = Dot(Vector3D{ nearest - i_point }, Vector3D{ nearest - i_point });

    return std::sqrt(dist_sqr);
}

double Distance(const Point3D& i_point1, const Point3D& i_point2)
{
    return std::sqrt(DistanceSqr(i_point1, i_point2));
}

bool TriangleWithBBoxIntersection(const Triangle& i_triangle, const BoundingBox& i_bbox)
{
    if(i_bbox.ContainsPoint(i_triangle.GetPoint(0)) || i_bbox.ContainsPoint(i_triangle.GetPoint(1)) || i_bbox.ContainsPoint(i_triangle.GetPoint(2)))
    {
        return true;
    }

    auto bbox_center = (i_bbox.GetMin() + i_bbox.GetMax()) / 2;
    auto half_delta = (i_bbox.GetMax() - i_bbox.GetMin()) / 2;

    // translated triangle points
    auto tr_point0 = i_triangle.GetPoint(0) - bbox_center;
    auto tr_point1 = i_triangle.GetPoint(1) - bbox_center;
    auto tr_point2 = i_triangle.GetPoint(2) - bbox_center;

    auto edge0 = tr_point1 - tr_point0;
    auto edge1 = tr_point2 - tr_point1;
    auto edge2 = tr_point0 - tr_point2;

    if (!_AxisTestX(edge0.GetZ(), edge0.GetY(), tr_point0, tr_point2, half_delta))
        return false;
    if (!_AxisTestY(edge0.GetZ(), edge0.GetX(), tr_point0, tr_point2, half_delta))
        return false;
    if (!_AxisTestZ(edge0.GetY(), edge0.GetX(), tr_point1, tr_point2, half_delta))
        return false;

    if (!_AxisTestX(edge1.GetZ(), edge1.GetY(), tr_point0, tr_point2, half_delta))
        return false;
    if (!_AxisTestY(edge1.GetZ(), edge1.GetX(), tr_point0, tr_point2, half_delta))
        return false;
    if (!_AxisTestZ(edge1.GetY(), edge1.GetX(), tr_point0, tr_point1, half_delta))
        return false;

    if (!_AxisTestX(edge2.GetZ(), edge2.GetY(), tr_point0, tr_point1, half_delta))
        return false;
    if (!_AxisTestY(edge2.GetZ(), edge2.GetX(), tr_point0, tr_point1, half_delta))
        return false;
    if (!_AxisTestZ(edge2.GetY(), edge2.GetX(), tr_point1, tr_point2, half_delta))
        return false;

    double min = 0, max = 0;

    _UpdateMinMax(tr_point0.GetX(), tr_point1.GetX(), tr_point2.GetX(), min, max);
    min -= DBL_EPSILON;
    max += DBL_EPSILON;
    if (min > half_delta.GetX() || max < -half_delta.GetX())
        return false;

    _UpdateMinMax(tr_point0.GetY(), tr_point1.GetY(), tr_point2.GetY(), min, max);
    min -= DBL_EPSILON;
    max += DBL_EPSILON;
    if (min > half_delta.GetY() || max < -half_delta.GetY())
        return false;

    _UpdateMinMax(tr_point0.GetZ(), tr_point1.GetZ(), tr_point2.GetZ(), min, max);
    min -= DBL_EPSILON;
    max += DBL_EPSILON;
    if (min > half_delta.GetZ() || max < -half_delta.GetZ())
        return false;

    auto normal = Cross(Vector3D(edge0), Vector3D(edge1));
    if (!_PlaneBoxOverlap(tr_point0, normal, half_delta))
        return false;

    return true;
}

void ExtrudeInplace(BoundingBox& i_bbox, double i_offset)
{
    assert(i_offset >= 0);
    Point3D offset_point(i_offset, i_offset, i_offset);
    i_bbox.AddPoint(i_bbox.GetMin() - offset_point);
    i_bbox.AddPoint(i_bbox.GetMax() + offset_point);
}

PointTriangleRelativeLocationResult GetPointTriangleRelativeLocation(const Triangle& i_triangle, const Point3D& i_point)
{
    const Plane plane(i_triangle.GetPoint(0), i_triangle.GetNormal());
    const auto location_result = plane.LocatePoint(i_point);

    switch (location_result)
    {
    case Plane::PointLocationResult::Above:
        return PointTriangleRelativeLocationResult::Above;
    case Plane::PointLocationResult::Below:
        return PointTriangleRelativeLocationResult::Below;
    case Plane::PointLocationResult::OnPlane:
        return PointTriangleRelativeLocationResult::OnSamePlane;
    default:
        assert(false);
        break;
    }
    
    return PointTriangleRelativeLocationResult::Above;
}
