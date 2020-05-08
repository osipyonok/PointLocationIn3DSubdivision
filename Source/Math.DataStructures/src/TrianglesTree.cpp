#include "Math.DataStructures/TrianglesTree.h"

void NearestTriangleApproximationFunctor::operator()(TrianglesTreeNode& i_root, Triangle*& io_triangle, const Point3D& i_point)
{
    if (!i_root.GetInfo().m_bbox.ContainsPoint(i_point))
        return;

    const bool is_leaf = !i_root.HasLeftChild() && !i_root.HasRightChild();
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

void BuildTrianglesTreeFunctor::operator()(TrianglesTreeNode& i_root, std::vector<Triangle*> i_triangles)
{
    if (!i_root.GetInfo().m_bbox.IsValid())
    {
        for (auto p_triangle : i_triangles)
        {
            i_root.GetInfo().m_bbox.AddPoint(p_triangle->GetPoint(0));
            i_root.GetInfo().m_bbox.AddPoint(p_triangle->GetPoint(1));
            i_root.GetInfo().m_bbox.AddPoint(p_triangle->GetPoint(2));
        }
    }

    if (i_triangles.size() <= _triangles_per_leaf_threshold)
    {
        i_root.GetInfo().m_triangles.swap(i_triangles);
        return;
    }

    const auto& root_bbox = i_root.GetInfo().m_bbox;

    short longest_dim = -1;
    if (true)
    {
        double length = std::numeric_limits<double>::lowest();
        for (auto dim = 0u; dim < 3; ++dim)
        {
            auto current_length = root_bbox.GetDelta(dim);
            if (current_length >= length)
            {
                length = current_length;
                longest_dim = dim;
            }
        }
    }
    Q_ASSERT(longest_dim != -1);

    // can be done faster by using O(n) median finding algo
    std::sort(i_triangles.begin(), i_triangles.end(), [longest_dim](Triangle* ip_tr1, Triangle* ip_tr2)
    {
        auto first = ip_tr1->GetPoint(0).Get(longest_dim) + ip_tr1->GetPoint(1).Get(longest_dim) + ip_tr1->GetPoint(2).Get(longest_dim);
        auto second = ip_tr2->GetPoint(0).Get(longest_dim) + ip_tr2->GetPoint(1).Get(longest_dim) + ip_tr2->GetPoint(2).Get(longest_dim);
        return first < second;
    });

    auto p_median_triangle = i_triangles[i_triangles.size() / 2];
    auto center = (p_median_triangle->GetPoint(0) + p_median_triangle->GetPoint(1) + p_median_triangle->GetPoint(2)) / 3;
    auto koef = (center[longest_dim] - root_bbox.GetMin().Get(longest_dim)) / root_bbox.GetDelta(longest_dim);

    BoundingBox left_bbox;
    left_bbox.AddPoint(root_bbox.GetMin());
    left_bbox.AddPoint(root_bbox.GetMin() + Point3D((longest_dim == 0 ? koef : 1.0) * root_bbox.GetDelta(0),
                                                    (longest_dim == 1 ? koef : 1.0) * root_bbox.GetDelta(1),
                                                    (longest_dim == 2 ? koef : 1.0) * root_bbox.GetDelta(2)));
    BoundingBox right_bbox;
    right_bbox.AddPoint(root_bbox.GetMax());
    right_bbox.AddPoint(root_bbox.GetMax() - Point3D((longest_dim == 0 ? 1 - koef : 1.0) * root_bbox.GetDelta(0),
                                                     (longest_dim == 1 ? 1 - koef : 1.0) * root_bbox.GetDelta(1),
                                                     (longest_dim == 2 ? 1 - koef : 1.0) * root_bbox.GetDelta(2)));

    std::vector<Triangle*> left_triangles;
    std::vector<Triangle*> right_triangles;
    for (auto p_triangle : i_triangles)
    {
        if (TriangleWithBBoxIntersection(*p_triangle, left_bbox))
            left_triangles.emplace_back(p_triangle);
        if (TriangleWithBBoxIntersection(*p_triangle, right_bbox))
            right_triangles.emplace_back(p_triangle);
    }

    if (left_triangles.empty())
    {
        i_root.GetInfo().m_triangles.swap(left_triangles);
        return;
    }
    if (right_triangles.empty())
    {
        i_root.GetInfo().m_triangles.swap(right_triangles);
        return;
    }
    if (left_triangles.size() == i_triangles.size() || right_triangles.size() == i_triangles.size())
    {
        i_root.GetInfo().m_triangles.swap(i_triangles);
        return;
    }

    i_triangles.clear();

    i_root.GetLeftChild().GetInfo().m_bbox = left_bbox;
    i_root.GetRightChild().GetInfo().m_bbox = right_bbox;

    this->operator()(i_root.GetLeftChild(), left_triangles);
    this->operator()(i_root.GetRightChild(), right_triangles);
}
