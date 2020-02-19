#include "Math.Core/CommonUtilities.h"

#include "Math.Core/Point3D.h"
#include "Math.Core/Triangle.h"
#include "Math.Core/Vector3D.h"
#include "Math.Core/VectorUtilities.h"

#include <cmath>
#include <utility>


double Distance(const Point3D& i_point, const Triangle& i_triangle)
{
    auto diff = i_point - *i_triangle.GetPoint(0);
    auto edge1 = *i_triangle.GetPoint(1) - *i_triangle.GetPoint(0);
    auto edge2 = *i_triangle.GetPoint(2) - *i_triangle.GetPoint(0);

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

    auto nearest = *i_triangle.GetPoint(0) + point.first * edge1 + point.second * edge2;
    auto dist_sqr = Dot(Vector3D{ nearest - i_point }, Vector3D{ nearest - i_point });

    return std::sqrt(dist_sqr);
}
