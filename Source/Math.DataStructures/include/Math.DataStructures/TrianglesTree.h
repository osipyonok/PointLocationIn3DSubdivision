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

struct MATH_DATASTRUCTURES_API BuildTrianglesTreeFunctor
{
    static constexpr auto _triangles_per_leaf_threshold = 100u;

    void operator()(TrianglesTreeNode& i_root, std::vector<Triangle*> i_triangles);
};

struct MATH_DATASTRUCTURES_API NearestTriangleApproximationFunctor
{
    void operator()(TrianglesTreeNode& i_root, Triangle*& io_triangle, const Point3D& i_point);
};

using TrianglesTree = GenericKDTree<TrianglesTreeInfo, BuildTrianglesTreeFunctor, NearestTriangleApproximationFunctor>;
