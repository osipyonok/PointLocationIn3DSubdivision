#include "Math.Core/CommonUtilities.h"

#include "Math.Core/BoundingBox.h"
#include "Math.Core/Point3D.h"
#include "Math.Core/Triangle.h"
#include "Math.Core/Vector3D.h"
#include "Math.Core/VectorUtilities.h"

#include <cmath>
#include <tuple>
#include <utility>


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
        distance = std::max(distance, Distance(i_point, triangle));
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
