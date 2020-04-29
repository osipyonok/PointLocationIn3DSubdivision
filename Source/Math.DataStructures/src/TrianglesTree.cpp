#include "Math.DataStructures/TrianglesTree.h"

void NearestTriangleApproximationFunctor::operator()(TrianglesTreeNode& i_root, Triangle*& io_triangle, const Point3D& i_point)
{
    if (!i_root.GetInfo().m_bbox.ContainsPoint(i_point))
        return;

    const bool is_leaf = (!i_root.HasLeftChild() || (i_root.HasLeftChild() && i_root.GetLeftChild().GetInfo().m_triangles.empty()))
        && (!i_root.HasRightChild() || (i_root.HasRightChild() && i_root.GetRightChild().GetInfo().m_triangles.empty()));
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
