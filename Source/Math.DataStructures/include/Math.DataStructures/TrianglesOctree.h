#pragma once

#include <Math.DataStructures/API.h>

#include <Math.DataStructures/GenericOctree.h>

#include <Math.Core/BoundingBox.h>
#include <Math.Core/CommonUtilities.h>
#include <Math.Core/Point3D.h>
#include <Math.Core/Triangle.h>

#include <QStringView>

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
    struct MATH_DATASTRUCTURES_API TrianglesOcTreeBoundingBoxUpdater
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

    struct MATH_DATASTRUCTURES_API TrianglesOcTreeBuildFunctor
    {
        void operator()(TrianglesOcTreeNode& io_root, std::vector<TriangleWithMeshTag> i_triangles);

    private:
        int m_depth = 0;
        std::vector<TrianglesOcTreeNode*> m_empty_nodes_to_fill_in;
    };

    struct MATH_DATASTRUCTURES_API TrianglesOcTreeQueryFunctor
    {
        void operator()(const TrianglesOcTreeNode& i_root, TriangleOcTreeQueryResult& o_result, const Point3D& i_point);
    };
}

using TrianglesOcTree = GenericOcTree<TrianglesOcTreeInfo,
                                      Details::TrianglesOcTreeBuildFunctor,
                                      Details::TrianglesOcTreeQueryFunctor,
                                      Details::TrianglesOcTreeBoundingBoxUpdater
                                     >;
