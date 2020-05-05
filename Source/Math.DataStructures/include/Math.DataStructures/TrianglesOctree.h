#pragma once

#include <Math.DataStructures/API.h>

#include <Math.DataStructures/GenericOctree.h>

#include <Math.Core/BoundingBox.h>
#include <Math.Core/CommonUtilities.h>
#include <Math.Core/Point3D.h>
#include <Math.Core/Triangle.h>

#include <QStringView>

#include <boost/optional.hpp>

#include <utility>
#include <vector>

using TriangleWithMeshTag = std::pair<Triangle*, QStringView>;

struct TrianglesOcTreeInfo
{
    BoundingBox m_triangles_bbox;
    std::vector<TriangleWithMeshTag> m_triangles;

    // these two guys below are valid only if it's an empty leaf node
    bool m_is_empty_leaf = false;
    QStringView m_fully_inside_mesh; // if (m_is_empty_leaf && m_fully_inside_mesh.isNull()) means that this leaf is located outside
};
using TrianglesOcTreeNode = OcTreeNode<TrianglesOcTreeInfo>;

struct TriangleOcTreeQueryResult
{
    enum Status { Outside, Inside };

    QStringView m_mesh_name;
    Status m_status = Outside;
};

namespace Details
{
    struct TrianglesOcTreeBoundingBoxUpdater
    {
        void operator()(BoundingBox& io_bbox, const std::vector<TriangleWithMeshTag>& i_triangles)
        {
            std::vector<Triangle*> triangles;
            triangles.reserve(i_triangles.size());
            for (auto& tr : i_triangles)
                triangles.emplace_back(tr.first);
            this->operator()(io_bbox, triangles);
        }

        void operator()(BoundingBox& io_bbox, const std::vector<Triangle*>& i_triangles)
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
        void operator()(TrianglesOcTreeNode& io_root, std::vector<TriangleWithMeshTag> i_triangles)
        {
            ScopedStateRestorer<int>  depth_state_destorer(m_depth);
            ++m_depth;
            if (m_depth == 1)
                m_empty_nodes_to_fill_in.clear();

            if (i_triangles.empty()) // an empty leaf
            {
                m_empty_nodes_to_fill_in.emplace_back(&io_root);
                return;
            }

            if (i_triangles.size() < 30) // let it be a normal leaf
            {
                auto& info = io_root.GetInfo();
                TrianglesOcTreeBoundingBoxUpdater updater;
                for (auto triangle : i_triangles)
                {
                    std::invoke(updater, info.m_triangles_bbox, std::vector<Triangle*>{ triangle.first });
                }
                info.m_triangles = std::move(i_triangles);
                
                return;
            }

            const auto& root_bbox = io_root.GetBoundingBox();

            std::vector<TriangleWithMeshTag> child_triangles[8];
            std::vector<BoundingBox> child_bboxes;
            for (size_t i = 0; i < 8; ++i)
            {
                auto child_bbox = io_root.GetPotentialChildBBox(i); 
                child_bboxes.emplace_back(child_bbox);

                for (size_t j = 0; j < i_triangles.size(); ++j)
                {
                    if (TriangleWithBBoxIntersection(*i_triangles[j].first, child_bbox))
                    {
                        child_triangles[i].emplace_back(i_triangles[j]);
                    }
                }
            }

            for (size_t i = 0; i < 8; ++i)
            {
                if (child_triangles[i].size() == i_triangles.size())
                {
                    auto& info = io_root.GetInfo();
                    TrianglesOcTreeBoundingBoxUpdater updater;
                    for (auto triangle : i_triangles)
                    {
                        std::invoke(updater, info.m_triangles_bbox, std::vector<Triangle*>{ triangle.first });
                    }
                    info.m_triangles = std::move(i_triangles);

                    return;
                }
            }

            i_triangles.clear();

            for (size_t i = 0; i < 8; ++i)
            {
                auto& child_node = *io_root.GetOrCreateChild(i);
                this->operator()(child_node, std::move(child_triangles[i]));

                auto& child_info = child_node.GetInfo();
                auto& child_triangles_bbox = child_info.m_triangles_bbox;
                if (child_triangles_bbox.IsValid())
                {
                    io_root.GetInfo().m_triangles_bbox.AddPoint(child_triangles_bbox.GetMin());
                    io_root.GetInfo().m_triangles_bbox.AddPoint(child_triangles_bbox.GetMax());
                }
            }

            if (m_depth == 1) // all non-empty leafs were built, lets fill the empty ones
            {
                while (!m_empty_nodes_to_fill_in.empty())
                {
                    auto p_empty_leaf = m_empty_nodes_to_fill_in.back();
                    m_empty_nodes_to_fill_in.pop_back();

                    bool the_node = p_empty_leaf->GetBoundingBox().ContainsPoint({ 0,0,0 });

                    p_empty_leaf->GetInfo().m_is_empty_leaf = true;

                    auto p_parent = p_empty_leaf->GetParent();
                    auto parent_triangles = _CollectNodesTriangles(p_parent);

                    auto min_point = p_empty_leaf->GetBoundingBox().GetMin();
                    auto it = std::min_element(parent_triangles.begin(), parent_triangles.end(), [min_point](const TriangleWithMeshTag& i_triangle1, const TriangleWithMeshTag& i_triangle2)
                    {
                        return Distance(min_point, *i_triangle1.first)
                             < Distance(min_point, *i_triangle2.first);
                    });
                    assert(it != parent_triangles.end());

                    auto loc_result = GetPointTriangleRelativeLocation(*it->first, min_point);
                    if (loc_result == PointTriangleRelativeLocationResult::Below
                     || loc_result == PointTriangleRelativeLocationResult::OnSamePlane)
                    {
                        p_empty_leaf->GetInfo().m_fully_inside_mesh = it->second;
                    }
                    else
                    {
                        p_empty_leaf->GetInfo().m_fully_inside_mesh = QStringView();
                    }

                }
            }
        }

    private:
        static std::vector<TriangleWithMeshTag> _CollectNodesTriangles(const TrianglesOcTreeNode* ip_node)
        {
            if (!ip_node->GetChild(0)) // if it's a leaf (of any kind)
                return ip_node->GetInfo().m_triangles;

            std::vector<TriangleWithMeshTag> result;
            for (size_t i = 0; i < 8; ++i)
            {
                if (auto p_child = ip_node->GetChild(i))
                {
                    auto child_triangles = _CollectNodesTriangles(p_child);
                    result.reserve(result.size() + child_triangles.size());
                    result.insert(result.end(), child_triangles.begin(), child_triangles.end());
                }
            }
            return std::move(result);
        }

        int m_depth = 0;
        std::vector<TrianglesOcTreeNode*> m_empty_nodes_to_fill_in;
    };

    struct TrianglesOcTreeQueryFunctor
    {
        void operator()(const TrianglesOcTreeNode& i_root, TriangleOcTreeQueryResult& o_result, const Point3D& i_point)
        {
            auto& info = i_root.GetInfo();
            if (!i_root.GetBoundingBox().ContainsPoint(i_point))
                return;

            bool is_leaf = !i_root.GetChild(0);

            if (is_leaf && info.m_is_empty_leaf)
            {
                if (info.m_fully_inside_mesh.isNull())
                {
                    o_result.m_status = TriangleOcTreeQueryResult::Outside;
                    o_result.m_mesh_name = QStringView();
                }
                else
                {
                    o_result.m_status = TriangleOcTreeQueryResult::Inside;
                    o_result.m_mesh_name = info.m_fully_inside_mesh;
                }

                return;
            }

            if (is_leaf)
            {
                Q_ASSERT(!info.m_triangles.empty());
                auto nearest_distance = std::numeric_limits<double>::max();
                boost::optional<TriangleWithMeshTag> nearest_triangle;
                for (auto triangle : info.m_triangles)
                {
                    auto current_distance = Distance(i_point, *triangle.first);
                    if (current_distance < nearest_distance)
                    {
                        nearest_distance = current_distance;
                        nearest_triangle = triangle;
                    }
                }

                if (nearest_triangle.is_initialized())
                {
                    auto loc = GetPointTriangleRelativeLocation(*nearest_triangle->first, i_point);
                    if (loc == PointTriangleRelativeLocationResult::Below
                     || loc == PointTriangleRelativeLocationResult::OnSamePlane)
                    {
                        o_result.m_mesh_name = nearest_triangle->second;
                        o_result.m_status = TriangleOcTreeQueryResult::Inside;
                    }
                    else
                    {
                        o_result.m_mesh_name = QStringView();
                        o_result.m_status = TriangleOcTreeQueryResult::Outside;
                    }
                }
                else
                {
                    o_result.m_mesh_name = QStringView();
                    o_result.m_status = TriangleOcTreeQueryResult::Outside;
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

            this->operator()(*i_root.GetChild(child_index), o_result, i_point);
        }
    };
}

using TrianglesOcTree = GenericOcTree<TrianglesOcTreeInfo,
                                      Details::TrianglesOcTreeBuildFunctor,
                                      Details::TrianglesOcTreeQueryFunctor,
                                      Details::TrianglesOcTreeBoundingBoxUpdater
                                     >;
