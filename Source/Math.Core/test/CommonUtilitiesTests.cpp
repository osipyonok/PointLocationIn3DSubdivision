#include <gtest/gtest.h>

#include <Math.Core/CommonUtilities.h>

#include <Math.Core/Point3D.h>
#include <Math.Core/Triangle.h>

#include <cmath>

using namespace ::testing;

TEST(Distance, PointIsInsideTriangle)
{
    Point3D triangle_points[3] = { {0, 0, 0}, {2, 0, 0}, {0, 2, 0} };
    Triangle triangle(triangle_points[0], triangle_points[1], triangle_points[2]);

    Point3D point(0.5, 0.5, 0);

    EXPECT_DOUBLE_EQ(Distance(point, triangle), 0);
}

TEST(Distance, ProjectionIsInsideTriangle)
{
    Point3D triangle_points[3] = { {0, 0, 0}, {2, 0, 0}, {0, 2, 0} };
    Triangle triangle(triangle_points[0], triangle_points[1], triangle_points[2]);

    Point3D point(0.5, 0.5, 3.2514);

    EXPECT_DOUBLE_EQ(Distance(point, triangle), 3.2514);
}

TEST(Distance, PointAndTriangleAreOnSamePlaneAndPerpendicularIsOnEdge)
{
    Point3D triangle_points[3] = { {0, 0, 0}, {2, 0, 0}, {0, 2, 0} };
    Triangle triangle(triangle_points[0], triangle_points[1], triangle_points[2]);

    Point3D point(2, 2, 0);

    EXPECT_DOUBLE_EQ(Distance(point, triangle), std::sqrt(2));
}

TEST(Distance, PointAndTriangleAreOnSamePlaneAndPerpendicularIsOnEdge_ShouldNotDependOnEdge)
{
    Point3D triangle_points[3] = { {0, 0, 0}, {2, 0, 0}, {0, 2, 0} };
    Triangle triangle(triangle_points[0], triangle_points[1], triangle_points[2]);

    Point3D point01(1, -1, 0);
    EXPECT_DOUBLE_EQ(Distance(point01, triangle), 1);

    Point3D point12(3, 3, 0);
    EXPECT_DOUBLE_EQ(Distance(point12, triangle), 2 * std::sqrt(2));

    Point3D point20(-1, 1, 0);
    EXPECT_DOUBLE_EQ(Distance(point20, triangle), 1);
}

TEST(Distance, PointAndTriangleAreOnSamePlaneButPerpendicularIsNotOnEdge_ShouldNotDependOnEdge)
{
    Point3D triangle_points[3] = { {0, 0, 0}, {2, 0, 0}, {0, 2, 0} };
    Triangle triangle(triangle_points[0], triangle_points[1], triangle_points[2]);

    Point3D point_to_0(-1, -1, 0);
    EXPECT_DOUBLE_EQ(Distance(point_to_0, triangle), std::sqrt(2));

    Point3D point_to_1(3, -1, 0);
    EXPECT_DOUBLE_EQ(Distance(point_to_1, triangle), std::sqrt(2));

    Point3D point_to_2(-1, 3, 0);
    EXPECT_DOUBLE_EQ(Distance(point_to_2, triangle), std::sqrt(2));
}

TEST(Distance, PointAndTriangleAreNotOnSamePlaneAndPerpendicularIsNotOnEdge)
{
    Point3D triangle_points[3] = { {0, 0, 0}, {2, 0, 0}, {0, 2, 0} };
    Triangle triangle(triangle_points[0], triangle_points[1], triangle_points[2]);

    Point3D point(4, 4, 4);

    EXPECT_DOUBLE_EQ(Distance(point, triangle), std::sqrt(34));
}

TEST(Distance, PointAndTriangleAreNotOnSamePlaneAndPerpendicularIsNotOnEdge_ShouldNotDependOnEdge)
{
    Point3D triangle_points[3] = { {1, 1, 0}, {3, 3, 0}, {2, 2, 2} };
    Triangle triangle(triangle_points[0], triangle_points[1], triangle_points[2]);

    Point3D point_to_0(0, 0, -1);
    EXPECT_DOUBLE_EQ(Distance(point_to_0, triangle), std::sqrt(3));

    Point3D point_to_1(4, 4, -1);
    EXPECT_DOUBLE_EQ(Distance(point_to_1, triangle), std::sqrt(3));

    Point3D point_to_2(3, 1, 5);
    EXPECT_DOUBLE_EQ(Distance(point_to_2, triangle), std::sqrt(11));
}

