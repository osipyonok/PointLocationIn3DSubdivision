#pragma once

#include <Math.DataStructures/API.h>

#include <Math.DataStructures/GenericOctree.h>

#include <Math.Core/BoundingBox.h>
#include <Math.Core/CommonUtilities.h>
#include <Math.Core/Point3D.h>
#include <Math.Core/Triangle.h>

#include <vector>

struct TrianglesOcTreeInfo
{
    BoundingBox m_triangles_bbox;
    std::vector<Triangle*> m_triangles;
};
using TrianglesOcTreeNode = OcTreeNode<TrianglesOcTreeInfo>;

namespace Details
{
    struct TrianglesOcTreeBoundingBoxUpdater
    {
        void operator()(BoundingBox& io_bbox, const std::vector<Triangle*> i_triangles)
        {
            for (auto p_triangle : i_triangles)
            {
                if (!p_triangle)
                    return;

                io_bbox.AddPoint(p_triangle->GetPoint(0));
                io_bbox.AddPoint(p_triangle->GetPoint(1));
                io_bbox.AddPoint(p_triangle->GetPoint(2));
            }
        }
    };

    struct TrianglesOcTreeBuildFunctor
    {
        void operator()(TrianglesOcTreeNode& io_root, std::vector<Triangle*> i_triangles)
        {
            if (i_triangles.size() < 30) // let it be leaf
            {
                auto& info = io_root.GetInfo();
                TrianglesOcTreeBoundingBoxUpdater updater;
                for (auto p_triangle : i_triangles)
                {
                    std::invoke(updater, info.m_triangles_bbox, std::vector<Triangle*>{ p_triangle });
                }
                info.m_triangles = std::move(i_triangles);
                
                return;
            }

            const auto& root_bbox = io_root.GetBoundingBox();

            for (size_t i = 0; i < 8; ++i)
            {
                auto& child_node = *io_root.GetOrCreateChild(i);
                auto& child_bbox = child_node.GetBoundingBox();

                std::vector<Triangle*> child_triangles;
                for (size_t j = 0; j < i_triangles.size(); ++j)
                {
                    if (TriangleWithBBoxIntersection(*i_triangles[j], child_bbox))
                    {
                        child_triangles.emplace_back(i_triangles[j]);
                    }
                }

                this->operator()(child_node, std::move(child_triangles));

                auto& child_info = child_node.GetInfo();
                auto& child_triangles_bbox = child_info.m_triangles_bbox;
                if (child_triangles_bbox.IsValid())
                {
                    io_root.GetInfo().m_triangles_bbox.AddPoint(child_triangles_bbox.GetMin());
                    io_root.GetInfo().m_triangles_bbox.AddPoint(child_triangles_bbox.GetMax());
                }
            }
        }
    };

    struct TrianglesOcTreeQueryFunctor
    {
        void operator()(const TrianglesOcTreeNode& i_root, Triangle*& io_triangle, const Point3D& i_point)
        {
            auto& info = i_root.GetInfo();
            if (!info.m_triangles_bbox.ContainsPoint(i_point))
                return;

            bool is_leaf = !i_root.GetChild(0);
            if (is_leaf)
            {
                auto nearest_distance = io_triangle ? Distance(i_point, *io_triangle) : std::numeric_limits<double>::max();
                for (auto p_triangle : info.m_triangles)
                {
                    auto current_distance = Distance(i_point, *p_triangle);
                    if (current_distance < nearest_distance)
                    {
                        nearest_distance = current_distance;
                        io_triangle = p_triangle;
                    }
                }
                return;
            }

            auto& bbox = i_root.GetBoundingBox();
            auto center = (bbox.GetMin() + bbox.GetMax()) / 2;
            size_t child_index = 0;
            if (center.GetX() < i_point.GetX())
                child_index += 1;
            if (center.GetY() < i_point.GetY())
                child_index += 2;
            if (center.GetZ() < i_point.GetZ())
                child_index += 4;

            this->operator()(*i_root.GetChild(child_index), io_triangle, i_point);

            if (!io_triangle && should_check_for_nearest_node) // damn, can't locate point in it's own node, let's find the nearest non-emply leaf node then
            {
                should_check_for_nearest_node = false;
                const TrianglesOcTreeNode* p_nearest_node = nullptr;
                double distance = std::numeric_limits<double>::max();

                std::function<void(const TrianglesOcTreeNode&)> node_finder = [&](const TrianglesOcTreeNode& i_node)
                {
                    bool is_leaf = !i_node.GetChild(0);
                    if (!is_leaf)
                    {
                        for (size_t i = 0; i < 8; ++i)
                        {
                            node_finder(*i_node.GetChild(i));
                        }
                        return;
                    }

                    // this is a leaf
                    bool is_empty = i_node.GetInfo().m_triangles.empty();
                    if (is_empty)
                        return;

                    auto current_distance = Distance(i_point, i_node.GetBoundingBox());
                    if (current_distance < distance)
                    {
                        distance = current_distance;
                        p_nearest_node = &i_node;
                    }
                };
                node_finder(i_root);

                if (p_nearest_node)
                {
                    auto nearest_distance = std::numeric_limits<double>::max();
                    for (auto p_triangle : p_nearest_node->GetInfo().m_triangles)
                    {
                        auto current_distance = Distance(i_point, *p_triangle);
                        if (current_distance < nearest_distance)
                        {
                            nearest_distance = current_distance;
                            io_triangle = p_triangle;
                        }
                    }
                }
            }
        }

    private:
        bool should_check_for_nearest_node = true;
    };
}

using TrianglesOcTree = GenericOcTree<TrianglesOcTreeInfo,
                                      Details::TrianglesOcTreeBuildFunctor,
                                      Details::TrianglesOcTreeQueryFunctor,
                                      Details::TrianglesOcTreeBoundingBoxUpdater
                                     >;
