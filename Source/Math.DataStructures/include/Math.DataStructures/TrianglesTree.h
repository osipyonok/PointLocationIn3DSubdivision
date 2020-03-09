#pragma once

#include <Math.DataStructures/API.h>

#include <Math.DataStructures/GenericKDTree.h>

#include <Math.Core/BoundingBox.h>
#include <Math.Core/CommonUtilities.h>
#include <Math.Core/Point3D.h>
#include <Math.Core/Triangle.h>

#include <QtGlobal>

#include <vector>

struct TrianglesTreeInfo
{
    std::vector<Triangle*> m_triangles;
    BoundingBox m_bbox;
};

using TrianglesTreeNode = KDTreeNode<TrianglesTreeInfo>;

struct BuildTrianglesTreeFunctor
{
    static constexpr auto _triangles_per_leaf_threshold = 20u;

    void operator()(TrianglesTreeNode& i_root, std::vector<Triangle*> i_triangles) // move this to cpp
    {
        for (auto p_triangle : i_triangles)
        {
            i_root.GetInfo().m_bbox.AddPoint(*p_triangle->GetPoint(0));
            i_root.GetInfo().m_bbox.AddPoint(*p_triangle->GetPoint(1));
            i_root.GetInfo().m_bbox.AddPoint(*p_triangle->GetPoint(2));
        }

        if (i_triangles.size() <= _triangles_per_leaf_threshold)
        {
            i_root.GetInfo().m_triangles.swap(i_triangles);
            return;
        }

        short longest_dim = -1;
        if (true)
        {
            double length = std::numeric_limits<double>::lowest();
            for (auto dim = 0u; dim < 3; ++dim)
            {
                auto current_length = i_root.GetInfo().m_bbox.GetDelta(dim);
                if (current_length >= length)
                {
                    length = current_length;
                    longest_dim = dim;
                }
            }
        }
        Q_ASSERT(longest_dim != -1);

        std::vector<Triangle*> left_triangles;
        std::vector<Triangle*> right_triangles;

        for (auto p_triangle : i_triangles)
        {
            auto value = p_triangle->GetPoint(0)->Get(longest_dim);
            value = std::max(value, p_triangle->GetPoint(1)->Get(longest_dim));
            value = std::max(value, p_triangle->GetPoint(2)->Get(longest_dim));

            if (value * 2 < i_root.GetInfo().m_bbox.GetDelta(longest_dim))
            {
                left_triangles.emplace_back(p_triangle);
            }
            else
            {
                right_triangles.emplace_back(p_triangle);
            }
        }

        Q_ASSERT(!left_triangles.empty());
        Q_ASSERT(!right_triangles.empty());

        i_triangles.clear();

        this->operator()(i_root.GetLeftChild(), std::move(left_triangles));
        this->operator()(i_root.GetRightChild(), std::move(right_triangles));
    }
};

struct NearestTriangleApproximationFunctor
{
    void operator()(TrianglesTreeNode& i_root, Triangle*& io_triangle, const Point3D& i_point)
    {
        if (!i_root.GetInfo().m_bbox.ContainsPoint(i_point))
            return;

        const bool is_leaf = i_root.HasLeftChild() || i_root.HasRightChild();
        if (!is_leaf)
        {
            Q_ASSERT(i_root.HasLeftChild());
            Q_ASSERT(i_root.HasRightChild());
            this->operator()(i_root.GetLeftChild(), io_triangle, i_point);
            this->operator()(i_root.GetRightChild(), io_triangle, i_point);
            return;
        }

        auto nearest_distance = io_triangle ? Distance(i_point, *io_triangle) : std::numeric_limits<double>::max();
        for (auto p_triangle : i_root.GetInfo().m_triangles)
        {
            auto current_distance = Distance(i_point, *p_triangle);
            if (current_distance < nearest_distance)
            {
                nearest_distance = current_distance;
                io_triangle = p_triangle;
            }
        }
    }
};

struct BruteforceInRadius
{
    void operator()(TrianglesTreeNode& i_root, Triangle*& io_triangle, const Point3D& i_point, double i_radius)
    {

    }
};
