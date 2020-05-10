#include "Math.DataStructures/TrianglesTree.h"


#include <functional>
#include <numeric>


namespace
{
    Triangle* _QuickMedian(const std::vector<Triangle*>& i_triangles, std::function<double(Triangle*)> i_value_getter)
    {
        if (i_triangles.empty())
        {
            Q_ASSERT(false);
            return nullptr;
        }

        std::vector<std::pair<Triangle*, double>> triangles_with_values;
        triangles_with_values.reserve(i_triangles.size());

        std::transform(i_triangles.begin(), i_triangles.end(), std::back_inserter(triangles_with_values), [i_value_getter](auto ip_triangle)
        {
            return std::make_pair(ip_triangle, i_value_getter(ip_triangle));
        });

        std::function<size_t(const std::vector<size_t>&, size_t)> quick_median_impl = [&](const std::vector<size_t>& i_indexes, size_t i_k) -> size_t
        {
            if (i_indexes.empty())
            {
                Q_ASSERT(false);
                return std::numeric_limits<size_t>::max();
            }

            if (i_indexes.size() == 1)
            {
                return i_indexes.front();
            }

            const double pivot_value = triangles_with_values[i_indexes[0]].second;
            std::vector<size_t> less;
            std::copy_if(i_indexes.begin() + 1, i_indexes.end(), std::back_inserter(less), [&](size_t i_index)
            {
                return triangles_with_values[i_index].second < pivot_value;
            });

            if (less.size() >= i_k)
            {
                return quick_median_impl(less, i_k);
            }

            std::vector<size_t> same;
            same.push_back(i_indexes.front());
            std::copy_if(i_indexes.begin() + 1, i_indexes.end(), std::back_inserter(same), [&](size_t i_index)
            {
                return triangles_with_values[i_index].second == pivot_value;
            });

            if (less.size() + same.size() >= i_k)
            {
                return i_indexes.front();
            }

            std::vector<size_t> greater;
            std::copy_if(i_indexes.begin() + 1, i_indexes.end(), std::back_inserter(greater), [&](size_t i_index)
            {
                return triangles_with_values[i_index].second > pivot_value;
            });

            auto new_k = i_k - less.size() - same.size();
            less.clear();// clean-up memory
            same.clear();

            return quick_median_impl(greater, new_k);
        };

        std::vector<size_t> indexes;
        indexes.resize(i_triangles.size());
        std::iota(indexes.begin(), indexes.end(), 0);

        auto median_index = quick_median_impl(indexes, i_triangles.size() / 2);
        return triangles_with_values[median_index].first;
    }
}

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

    auto p_median_triangle = _QuickMedian(i_triangles, [longest_dim](Triangle* ip_triangle)
    {
        return ip_triangle->GetPoint(0).Get(longest_dim) + ip_triangle->GetPoint(1).Get(longest_dim) + ip_triangle->GetPoint(2).Get(longest_dim);
    });
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
        i_root.GetInfo().m_triangles.swap(i_triangles);
        return;
    }
    if (right_triangles.empty())
    {
        i_root.GetInfo().m_triangles.swap(i_triangles);
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
