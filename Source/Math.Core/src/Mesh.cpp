#include "Math.Core/Mesh.h"

#include "Math.Core/MeshPoint.h"
#include "Math.Core/MeshTriangle.h"

#include <vector>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/random_access_index.hpp>

////////////////////////////////////////////////////

using boost::multi_index_container;
using namespace boost::multi_index;

namespace
{
    struct Edge
    {
        Edge(const Point3D& i_a, const Point3D& i_b)
            : m_first(std::min(i_a, i_b))
            , m_second(std::max(i_a, i_b))
        {
        }

        Point3D m_first, m_second;
    };

    size_t hash_value(const Edge& i_edge)
    {
        size_t hash = 0;
        boost::hash_combine(hash, i_edge.m_first);
        boost::hash_combine(hash, i_edge.m_second);
        return hash;
    }

    bool operator==(const Edge& i_lhs, const Edge& i_rhs)
    {
        return i_lhs.m_first == i_rhs.m_first && i_lhs.m_second == i_rhs.m_second;
    }

    struct TriangleContainer
    {
        MeshTriangle* AddTriangle(const Triangle& i_triangle);
        bool ContainsTriangle(const Triangle& i_triangle) const;
        MeshTriangle* GetTriangle(const Triangle& i_triangle) const;
        void RemoveTriangle(const Triangle& i_triangle);
        void RemoveTrianglesWithVertex(const Point3D& i_point);

        size_t GetTrianglesCount() const;
        MeshTriangle* GetTriangleAt(size_t i_index) const;

    private:
        enum class Coordinate { X = 0, Y = 1, Z = 2 };
        enum class EdgeNumber { First = 0, Second = 1, Third = 2 };

        struct TriangleTag;
        template<Coordinate coordinate>  struct VertexTag;
        template<EdgeNumber edge_number> struct NonOrientedEdgeTag;

        struct TriangleExtractor
        {
            using result_type = const Triangle&;

            result_type operator()(const std::unique_ptr<MeshTriangle>& ip_triangle) const
            {
                Q_ASSERT(ip_triangle);
                return *ip_triangle;
            }
        };

        template<Coordinate coordinate>
        struct VertexExtractor
        {
            using result_type = Point3D;

            result_type operator()(const std::unique_ptr<MeshTriangle>& ip_triangle) const
            {
                Q_ASSERT(ip_triangle);
                return ip_triangle->GetPoint(static_cast<int>(coordinate));
            }
        };

        template<EdgeNumber edge_number>
        struct NonOrientedEdgeExtractor
        {
            using result_type = Edge;

            result_type operator()(const std::unique_ptr<MeshTriangle>& ip_triangle) const
            {
                Q_ASSERT(ip_triangle);
                auto point1 = ip_triangle->GetPoint(static_cast<int>(edge_number));
                auto point2 = ip_triangle->GetPoint((static_cast<int>(edge_number) + 1) % 3);
                return result_type{ point1, point2 };
            }
        };

        template<EdgeNumber edge_number> // other triangle's edge number
        void _AddNeighbours(MeshTriangle& i_new_triangle);

        template<Coordinate coordinate> 
        void _RemoveTrianglesWithVertex(const Point3D& i_point);


        using TriangleContainerData = multi_index_container<
            std::unique_ptr<MeshTriangle>,
            indexed_by<
                random_access<>,
                hashed_unique<tag<TriangleTag>, TriangleExtractor>,
                hashed_non_unique<tag<VertexTag<Coordinate::X>>, VertexExtractor<Coordinate::X>>,
                hashed_non_unique<tag<VertexTag<Coordinate::Y>>, VertexExtractor<Coordinate::Y>>,
                hashed_non_unique<tag<VertexTag<Coordinate::Z>>, VertexExtractor<Coordinate::Z>>,
                hashed_non_unique<tag<NonOrientedEdgeTag<EdgeNumber::First>>,  NonOrientedEdgeExtractor<EdgeNumber::First>>,
                hashed_non_unique<tag<NonOrientedEdgeTag<EdgeNumber::Second>>, NonOrientedEdgeExtractor<EdgeNumber::Second>>,
                hashed_non_unique<tag<NonOrientedEdgeTag<EdgeNumber::Third>>,  NonOrientedEdgeExtractor<EdgeNumber::Third>>
            >
        >;

        TriangleContainerData m_data;
    };

    MeshTriangle* TriangleContainer::AddTriangle(const Triangle& i_triangle)
    {
        auto p_triangle = std::make_unique<MeshTriangle>(i_triangle.GetPoint(0), i_triangle.GetPoint(1), i_triangle.GetPoint(2));
        auto p_triangle_raw = p_triangle.get();

        _AddNeighbours<EdgeNumber::First>(*p_triangle);
        _AddNeighbours<EdgeNumber::Second>(*p_triangle);
        _AddNeighbours<EdgeNumber::Third>(*p_triangle);
        
        m_data.insert(m_data.end(), std::move(p_triangle));

        return p_triangle_raw;
    }

    bool TriangleContainer::ContainsTriangle(const Triangle& i_triangle) const
    {
        return GetTriangle(i_triangle);
    }

    MeshTriangle* TriangleContainer::GetTriangle(const Triangle& i_triangle) const
    {
        auto it = m_data.get<TriangleTag>().find(i_triangle);
        return it != m_data.get<TriangleTag>().end() ? it->get() : nullptr;
    }

    void TriangleContainer::RemoveTriangle(const Triangle& i_triangle)
    {
        Q_ASSERT(ContainsTriangle(i_triangle));
        auto it = m_data.get<TriangleTag>().find(i_triangle);
        if (it != m_data.get<TriangleTag>().end())
        {
            if (auto neighbour = (**it).GetNeighbour(0))
                GetTriangle(neighbour.get())->RemoveNeighbour(i_triangle);
            if (auto neighbour = (**it).GetNeighbour(1))
                GetTriangle(neighbour.get())->RemoveNeighbour(i_triangle);
            if (auto neighbour = (**it).GetNeighbour(2))
                GetTriangle(neighbour.get())->RemoveNeighbour(i_triangle);

            m_data.get<TriangleTag>().erase(it);
        }
    }

    void TriangleContainer::RemoveTrianglesWithVertex(const Point3D& i_point)
    {
        _RemoveTrianglesWithVertex<Coordinate::X>(i_point);
        _RemoveTrianglesWithVertex<Coordinate::Y>(i_point);
        _RemoveTrianglesWithVertex<Coordinate::Z>(i_point);
    }

    size_t TriangleContainer::GetTrianglesCount() const
    {
        return m_data.size();
    }

    MeshTriangle* TriangleContainer::GetTriangleAt(size_t i_index) const
    {
        return m_data[i_index].get();
    }

    template<TriangleContainer::EdgeNumber edge_number>
    void TriangleContainer::_AddNeighbours(MeshTriangle& i_new_triangle)
    {
        Edge edges[3] = 
        {
            { i_new_triangle.GetPoint(0), i_new_triangle.GetPoint(1) },
            { i_new_triangle.GetPoint(1), i_new_triangle.GetPoint(2) },
            { i_new_triangle.GetPoint(2), i_new_triangle.GetPoint(0) }
        };

        for (short i = 0; i < 3; ++i)
        {
            auto& edge = edges[i];
            auto range = m_data.get<NonOrientedEdgeTag<edge_number>>().equal_range(edge);
            Q_ASSERT(std::distance(range.first, range.second) <= 1);
            for (auto it = range.first; it != range.second; ++it)
            {
                m_data.get<NonOrientedEdgeTag<edge_number>>().modify(it, [&](const std::unique_ptr<MeshTriangle>& ip_triangle)
                {
                    ip_triangle->SetNeighbour(i_new_triangle, static_cast<short>(edge_number));
                    i_new_triangle.SetNeighbour(*ip_triangle, i);
                });
            }
        }
    }

    template<TriangleContainer::Coordinate coordinate>
    void TriangleContainer::_RemoveTrianglesWithVertex(const Point3D& i_point)
    {
        auto range = m_data.get<VertexTag<coordinate>>().equal_range(i_point);
        std::vector<Triangle*> triangles;
        for (auto it = range.first; it != range.second; ++it)
            triangles.emplace_back(it->get());

        for (auto p_triangle : triangles)
            RemoveTriangle(*p_triangle);
    }
}

struct Mesh::Impl
{
    std::vector<std::unique_ptr<MeshPoint>> m_points;
    TriangleContainer m_triangles;
};


Mesh::Mesh()
	: mp_impl(std::make_unique<Impl>())
{
}

Mesh::~Mesh() = default;

MeshPoint* Mesh::AddPoint(const Point3D& i_point)
{
    if (auto p_point = GetPoint(i_point))
        return p_point;

    return _AddPoint(i_point.GetX(), i_point.GetY(), i_point.GetZ());
}

MeshPoint* Mesh::AddPoint(double i_x, double i_y, double i_z)
{
    if (auto p_point = GetPoint(i_x, i_y, i_z))
        return p_point;

    return _AddPoint(i_x, i_y, i_z);
}

MeshTriangle* Mesh::AddTriangle(const Point3D& i_a, const Point3D& i_b, const Point3D& i_c)
{
    auto p_pnt1 = AddPoint(i_a);
    auto p_pnt2 = AddPoint(i_b);
    auto p_pnt3 = AddPoint(i_c);

    Q_ASSERT(p_pnt1);
    Q_ASSERT(p_pnt2);
    Q_ASSERT(p_pnt3);

    return mp_impl->m_triangles.AddTriangle(Triangle{ *p_pnt1, *p_pnt2, *p_pnt3 });
}

MeshPoint* Mesh::GetPoint(const Point3D& i_point) const
{
    for (const auto& p_point : mp_impl->m_points)
    {
        if (*p_point == i_point)
            return p_point.get();
    }
    return nullptr;
}

MeshPoint* Mesh::GetPoint(double i_x, double i_y, double i_z) const
{
    return GetPoint({ i_x, i_y, i_z });
}

MeshPoint* Mesh::GetPoint(size_t i_index) const
{
    Q_ASSERT(i_index >= 0 && i_index < GetPointsCount());
    return mp_impl->m_points[i_index].get();
}

MeshTriangle* Mesh::GetTriangleOrientationDependent(const Point3D& i_a, const Point3D& i_b, const Point3D& i_c) const
{
    return mp_impl->m_triangles.GetTriangle({ i_a, i_b, i_c });
}

MeshTriangle* Mesh::GetTriangleOrientationIndependent(const Point3D& i_a, const Point3D& i_b, const Point3D& i_c) const
{
    if (auto p_tr = GetTriangleOrientationDependent(i_a, i_b, i_c))
        return p_tr;
    return GetTriangleOrientationDependent(i_c, i_b, i_a);
}

MeshTriangle* Mesh::GetTriangle(size_t i_index) const
{
    Q_ASSERT(i_index >= 0 && i_index < GetTrianglesCount());
    return mp_impl->m_triangles.GetTriangleAt(i_index);
}

void Mesh::RemovePoint(const Point3D& i_point)
{
    mp_impl->m_triangles.RemoveTrianglesWithVertex(i_point);
    // todo: remove point from list
}

void Mesh::RemovePoint(double i_x, double i_y, double i_z)
{
    RemovePoint({ i_x, i_y, i_z });
}

void Mesh::RemoveTriangle(const Point3D& i_a, const Point3D& i_b, const Point3D& i_c)
{
    mp_impl->m_triangles.RemoveTriangle({ i_a, i_b, i_c });
}

size_t Mesh::GetPointsCount() const
{
    return mp_impl->m_points.size();
}

size_t Mesh::GetTrianglesCount() const
{
    return mp_impl->m_triangles.GetTrianglesCount();
}

MeshPoint* Mesh::_AddPoint(double i_x, double i_y, double i_z)
{
    mp_impl->m_points.emplace_back(std::make_unique<MeshPoint>(i_x, i_y, i_z));
    return mp_impl->m_points.back().get();
}
