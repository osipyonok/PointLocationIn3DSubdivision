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

    auto get_min_edge02 = [](double i_a11, double i_b1, std::pair<double, double>& o_point)
    {
        o_point.first = 0.;
        if (i_b1 >= 0)
            o_point.second = 0.;
        else if (i_a11 + i_b1 <= 0)
            o_point.second = 1.;
        else
            o_point.second = -i_b1 / i_a11;
    };

    auto get_min_edge12 = [](double i_a01, double i_a11, double i_b1, double i_f10, double i_f01, std::pair<double, double>& o_point)
    {
        auto h0 = i_a01 + i_b1 - i_f10;
        if (h0 < 0)
        {
            auto h1 = i_a11 + i_b1 - i_f01;
            o_point.second = h1 <= 0 ? 1. : (h0 / (h0 - h1));
        }
        else
        {
            o_point.second = 0.;
        }
        o_point.first = 1. - o_point.second;
    };

    auto get_min_inside = [](const std::pair<double, double>& i_p0, const std::pair<double, double>& i_p1, double h0, double h1, std::pair<double, double>& o_point)
    {
        auto z = h0 / (h0 - h1);
        o_point.first  = (1 - z) * i_p0.first  + z * i_p1.first;
        o_point.second = (1 - z) * i_p0.second + z * i_p1.second;
    };

    std::pair<double, double> point;

    if (f00 >= 0)
    {
        if (f01 < 0)
        {
            std::pair<double, double> point1 = { 0., f00 / (f00 - f01) };
            std::pair<double, double> point2 = { f01 / (f01 - f10), 1. - f01 / (f01 - f10) };

            auto h0 = (point2.second - point1.second) * (a11 * point1.second + b1);
            if (h0 >= 0)
            {
                get_min_edge02(a11, b1, point);
            }
            else
            {
                auto h1 = (point2.second - point1.second) * (a01 * point1.first + a11 * point1.second + b1);
                if (h1 <= 0)
                    get_min_edge12(a01, a11, b1, f10, f01, point);
                else
                    get_min_inside(point1, point2, h0, h1, point);
            }
        }
        else
        {
            get_min_edge02(a11, b1, point);
        }
    }
    else if (f01 <= 0)
    {
        if (f10 <= 0)
        {
            get_min_edge12(a01, a11, b1, f10, f01, point);
        }
        else
        {
            std::pair<double, double> point1 = { f00 / (f00 - f10), 0. };
            std::pair<double, double> point2 = { f01 / (f01 - f10), 1. - f01 / (f01 - f10) };
            auto h0 = point2.second * (a01 * point1.first + b1);
            if (h0 >= 0)
            {
                point = point1;
            }
            else
            {
                auto h1 = point2.second * (a01 * point2.first + a11 * point2.second + b1);
                if (h1 <= 0)
                {
                    get_min_edge12(a01, a11, b1, f10, f01, point);
                }
                else
                {
                    get_min_inside(point1, point2, h0, h1, point);
                }
            }
        }
    }
    else if (f10 <= 0)
    {
        std::pair<double, double> point1 = { 0, f00 / (f00 - f01) };
        std::pair<double, double> point2 = { f01 / (f01 - f10), 1. - f01 / (f01 - f10) };
        auto dt1 = point2.second - point1.second;
        auto h0 = dt1 * (a11 * point1.second + b1);
        if (h0 >= 0)
        {
            get_min_edge02(a11, b1, point);
        }
        else
        {
            auto h1 = dt1 * (a01 * point2.first + a11 * point2.second + b1);
            if (h1 <= 0)
            {
                get_min_edge12(a01, a11, b1, f10, f01, point);
            }
            else
            {
                get_min_inside(point1, point2, h0, h1, point);
            }
        }
    }
    else
    {
        std::pair<double, double> point1 = { f00 / (f00 - f10), 0 };
        std::pair<double, double> point2 = { 0, f00 / (f00 - f01) };
        auto h0 = point2.second * (a01 * point1.first + b1);
        if (h0 >= 0)
        {
            point = point1;
        }
        else
        {
            auto h1 = point2.second * (a11 * point2.second + b1);
            if (h1 <= 0)
            {
                get_min_edge02(a11, b1, point);
            }
            else
            {
                get_min_inside(point1, point2, h0, h1, point);
            }
        }
    }

    auto nearest = *i_triangle.GetPoint(0) + point.first * edge1 + point.second * edge2;
    auto dist_sqr = Dot(Vector3D{ nearest - i_point }, Vector3D{ nearest - i_point });
    // todo
    // https://www.geometrictools.com/GTEngine/Include/Mathematics/GteDistPointTriangle.h
    // http://web.mit.edu/ehliu/Public/Darmofal/DistancePoint3Triangle3.pdf
    return std::sqrt(dist_sqr);
}
