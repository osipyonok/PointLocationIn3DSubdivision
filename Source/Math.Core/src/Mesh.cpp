#include "Math.Core/Mesh.h"

#include "Math.Core/BoundingBox.h"
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
        TriangleHandle AddTriangle(const Triangle& i_triangle);
        bool ContainsTriangle(const Triangle& i_triangle) const;
        TriangleHandle GetTriangle(const Triangle& i_triangle) const;
        void RemoveTriangle(const Triangle& i_triangle);
        void RemoveTrianglesWithVertex(const Point3D& i_point);

        std::vector<TriangleHandle> GetTrianglesIncidentToEdge(const Point3D& i_first_point, const Point3D& i_second_point) const;

        void UpdateVertexPosition(const Point3D& i_old_vertex, const Point3D& i_new_vertex);

        size_t GetTrianglesCount() const;
        TriangleHandle GetTriangleAt(size_t i_index) const;

    private:
        using TrianglePtr = std::shared_ptr<MeshTriangle>;

        enum class VertexNumber { First = 0, Second = 1, Third = 2 };
        enum class EdgeNumber { First = 0, Second = 1, Third = 2 };

        struct TriangleTag;
        template<VertexNumber coordinate>  struct VertexTag;
        template<EdgeNumber edge_number> struct NonOrientedEdgeTag;

        struct TriangleExtractor
        {
            using result_type = const Triangle&;

            result_type operator()(const TrianglePtr& ip_triangle) const
            {
                Q_ASSERT(ip_triangle);
                return *ip_triangle;
            }
        };

        template<VertexNumber coordinate>
        struct VertexExtractor
        {
            using result_type = Point3D;

            result_type operator()(const TrianglePtr& ip_triangle) const
            {
                Q_ASSERT(ip_triangle);
                return ip_triangle->GetPoint(static_cast<int>(coordinate));
            }
        };

        template<EdgeNumber edge_number>
        struct NonOrientedEdgeExtractor
        {
            using result_type = Edge;

            result_type operator()(const TrianglePtr& ip_triangle) const
            {
                Q_ASSERT(ip_triangle);
                auto point1 = ip_triangle->GetPoint(static_cast<int>(edge_number));
                auto point2 = ip_triangle->GetPoint((static_cast<int>(edge_number) + 1) % 3);
                return result_type{ point1, point2 };
            }
        };

        template<EdgeNumber edge_number> // other triangle's edge number
        void _AddNeighbours(TriangleHandle i_new_triangle);

        template<EdgeNumber edge_number>
        std::vector<TriangleHandle> _GetTrianglesWithNonOrientedEdge(const Point3D& i_first_point, const Point3D& i_second_point) const;

        template<VertexNumber coordinate> 
        void _RemoveTrianglesWithVertex(const Point3D& i_point);

        template<VertexNumber vertex_number>
        void _UpdateVertexPosition(const Point3D& i_old_vertex, const Point3D& i_new_vertex);


        using TriangleContainerData = multi_index_container<
            TrianglePtr,
            indexed_by<
                random_access<>,
                hashed_unique<tag<TriangleTag>, TriangleExtractor>,
                hashed_non_unique<tag<VertexTag<VertexNumber::First>>, VertexExtractor<VertexNumber::First>>,
                hashed_non_unique<tag<VertexTag<VertexNumber::Second>>, VertexExtractor<VertexNumber::Second>>,
                hashed_non_unique<tag<VertexTag<VertexNumber::Third>>, VertexExtractor<VertexNumber::Third>>,
                hashed_non_unique<tag<NonOrientedEdgeTag<EdgeNumber::First>>,  NonOrientedEdgeExtractor<EdgeNumber::First>>,
                hashed_non_unique<tag<NonOrientedEdgeTag<EdgeNumber::Second>>, NonOrientedEdgeExtractor<EdgeNumber::Second>>,
                hashed_non_unique<tag<NonOrientedEdgeTag<EdgeNumber::Third>>,  NonOrientedEdgeExtractor<EdgeNumber::Third>>
            >
        >;

        TriangleContainerData m_data;
    };

    struct PointContainer
    {
        MeshPoint* AddPoint(const Point3D& i_point);
        MeshPoint* GetPoint(const Point3D& i_point) const;
        bool ContainsPoint(const Point3D& i_point) const;
        void RemovePoint(const Point3D& i_point);

        size_t GetPointsCount() const;
        MeshPoint* GetPointAt(size_t index) const;

        void UpdatePointCoordinates(const Point3D& i_old_coordinates, const Point3D& i_new_coordinates);

    private:
        struct PointTag;

        struct PointExtractor
        {
            using result_type = const Point3D&;

            result_type operator()(const std::unique_ptr<MeshPoint>& ip_point) const
            {
                Q_ASSERT(ip_point);
                return *ip_point;
            }
        };

        using PointContainerData = multi_index_container<
            std::unique_ptr<MeshPoint>,
            indexed_by<
                random_access<>,
                hashed_unique<tag<PointTag>, PointExtractor>
            >
        >;

        PointContainerData m_data;
    };

    TriangleHandle TriangleContainer::AddTriangle(const Triangle& i_triangle)
    {
        auto p_triangle = std::make_shared<MeshTriangle>(i_triangle.GetPoint(0), i_triangle.GetPoint(1), i_triangle.GetPoint(2));

        _AddNeighbours<EdgeNumber::First>(p_triangle);
        _AddNeighbours<EdgeNumber::Second>(p_triangle);
        _AddNeighbours<EdgeNumber::Third>(p_triangle);
        
        m_data.insert(m_data.end(), p_triangle);

        return p_triangle;
    }

    bool TriangleContainer::ContainsTriangle(const Triangle& i_triangle) const
    {
        return GetTriangle(i_triangle).lock() != nullptr;
    }

    TriangleHandle TriangleContainer::GetTriangle(const Triangle& i_triangle) const
    {
        auto it = m_data.get<TriangleTag>().find(i_triangle);
        return it != m_data.get<TriangleTag>().end() ? *it : nullptr;
    }

    void TriangleContainer::RemoveTriangle(const Triangle& i_triangle)
    {
        Q_ASSERT(ContainsTriangle(i_triangle));
        auto it = m_data.get<TriangleTag>().find(i_triangle);
        if (it != m_data.get<TriangleTag>().end())
        {
            for (auto neighbour_hangle : (**it).GetNeighbours(0))
            {
                if (auto p_neighbour = neighbour_hangle.lock())
                {
                    p_neighbour->RemoveNeighbour(i_triangle);
                }
            }

            for (auto neighbour_hangle : (**it).GetNeighbours(1))
            {
                if (auto p_neighbour = neighbour_hangle.lock())
                {
                    p_neighbour->RemoveNeighbour(i_triangle);
                }
            }

            for (auto neighbour_hangle : (**it).GetNeighbours(2))
            {
                if (auto p_neighbour = neighbour_hangle.lock())
                {
                    p_neighbour->RemoveNeighbour(i_triangle);
                }
            }

            m_data.get<TriangleTag>().erase(it);
        }
    }

    void TriangleContainer::RemoveTrianglesWithVertex(const Point3D& i_point)
    {
        _RemoveTrianglesWithVertex<VertexNumber::First>(i_point);
        _RemoveTrianglesWithVertex<VertexNumber::Second>(i_point);
        _RemoveTrianglesWithVertex<VertexNumber::Third>(i_point);
    }

    std::vector<TriangleHandle> TriangleContainer::GetTrianglesIncidentToEdge(const Point3D& i_first_point, const Point3D& i_second_point) const
    {
        auto by_first_edge  = _GetTrianglesWithNonOrientedEdge<EdgeNumber::First>(i_first_point, i_second_point);
        auto by_second_edge = _GetTrianglesWithNonOrientedEdge<EdgeNumber::Second>(i_first_point, i_second_point);
        auto by_third_edge  = _GetTrianglesWithNonOrientedEdge<EdgeNumber::Third>(i_first_point, i_second_point);

        std::vector<TriangleHandle> triangles;
        triangles.reserve(by_first_edge.size() + by_second_edge.size() + by_third_edge.size());

        std::copy(by_first_edge.begin(), by_first_edge.end(), std::back_inserter(triangles));
        std::copy(by_second_edge.begin(), by_second_edge.end(), std::back_inserter(triangles));
        std::copy(by_third_edge.begin(), by_third_edge.end(), std::back_inserter(triangles));

        return std::move(triangles);
    }

    void TriangleContainer::UpdateVertexPosition(const Point3D& i_old_vertex, const Point3D& i_new_vertex)
    {
        _UpdateVertexPosition<VertexNumber::First>(i_old_vertex, i_new_vertex);
        _UpdateVertexPosition<VertexNumber::Second>(i_old_vertex, i_new_vertex);
        _UpdateVertexPosition<VertexNumber::Third>(i_old_vertex, i_new_vertex);
    }

    size_t TriangleContainer::GetTrianglesCount() const
    {
        return m_data.size();
    }

    TriangleHandle TriangleContainer::GetTriangleAt(size_t i_index) const
    {
        return m_data[i_index];
    }

    template<TriangleContainer::EdgeNumber edge_number>
    void TriangleContainer::_AddNeighbours(TriangleHandle i_new_triangle)
    {
        auto& triangle = *i_new_triangle.lock();

        Edge edges[3] = 
        {
            { triangle.GetPoint(0), triangle.GetPoint(1) },
            { triangle.GetPoint(1), triangle.GetPoint(2) },
            { triangle.GetPoint(2), triangle.GetPoint(0) }
        };

        for (short i = 0; i < 3; ++i)
        {
            auto& edge = edges[i];
            auto range = m_data.get<NonOrientedEdgeTag<edge_number>>().equal_range(edge);
            //Q_ASSERT(std::distance(range.first, range.second) <= 1);
            for (auto it = range.first; it != range.second; ++it)
            {
                m_data.get<NonOrientedEdgeTag<edge_number>>().modify(it, [&](const TrianglePtr& ip_triangle)
                {
                    ip_triangle->SetNeighbour(i_new_triangle, static_cast<short>(edge_number));
                    i_new_triangle.lock()->SetNeighbour(ip_triangle, i);
                });
            }
        }
    }

    template<TriangleContainer::EdgeNumber edge_number>
    std::vector<TriangleHandle> TriangleContainer::_GetTrianglesWithNonOrientedEdge(const Point3D& i_first_point, const Point3D& i_second_point) const
    {
        auto range = m_data.get<NonOrientedEdgeTag<edge_number>>().equal_range(Edge{ i_first_point, i_second_point });
        std::vector<TriangleHandle> triangles;
        for (auto it = range.first; it != range.second; ++it)
        {
            triangles.push_back(*it);
        }
        return std::move(triangles);
    }

    template<TriangleContainer::VertexNumber coordinate>
    void TriangleContainer::_RemoveTrianglesWithVertex(const Point3D& i_point)
    {
        auto range = m_data.get<VertexTag<coordinate>>().equal_range(i_point);
        std::vector<TriangleHandle> triangles;
        for (auto it = range.first; it != range.second; ++it)
            triangles.emplace_back(*it);

        for (auto p_triangle : triangles)
            RemoveTriangle(*p_triangle.lock());
    }

    template<TriangleContainer::VertexNumber vertex_number>
    void TriangleContainer::_UpdateVertexPosition(const Point3D& i_old_vertex, const Point3D& i_new_vertex)
    {
        auto& container = m_data.get<VertexTag<vertex_number>>();
        for (auto it = container.find(i_old_vertex); it != container.end(); it = container.find(i_old_vertex))
        {
            container.modify(it, [&i_new_vertex](const TrianglePtr& ip_triangle)
            {
                ip_triangle->SetPoint(static_cast<int>(vertex_number), i_new_vertex);
            });
        }
    }

    MeshPoint* PointContainer::AddPoint(const Point3D& i_point)
    {
        auto p_point = std::make_unique<MeshPoint>(i_point.GetX(), i_point.GetY(), i_point.GetZ());
        auto p_point_raw = p_point.get();
        m_data.insert(m_data.end(), std::move(p_point));
        return p_point_raw;
    }

    MeshPoint* PointContainer::GetPoint(const Point3D& i_point) const
    {
        auto it = m_data.get<PointTag>().find(i_point);
        return it != m_data.get<PointTag>().end() ? it->get() : nullptr;
    }

    bool PointContainer::ContainsPoint(const Point3D& i_point) const
    {
        return GetPoint(i_point);
    }

    void PointContainer::RemovePoint(const Point3D& i_point)
    {
        m_data.get<PointTag>().erase(i_point);
    }

    size_t PointContainer::GetPointsCount() const
    {
        return m_data.size();
    }

    MeshPoint* PointContainer::GetPointAt(size_t index) const
    {
        return m_data[index].get();
    }

    void PointContainer::UpdatePointCoordinates(const Point3D& i_old_coordinates, const Point3D& i_new_coordinates)
    {
        auto it = m_data.get<PointTag>().find(i_old_coordinates);
        if (it == m_data.get<PointTag>().end())
            return;

        m_data.get<PointTag>().modify(it, [&](const std::unique_ptr<MeshPoint>& ip_point)
        {
            ip_point->SetX(i_new_coordinates.GetX());
            ip_point->SetY(i_new_coordinates.GetY());
            ip_point->SetZ(i_new_coordinates.GetZ());
        });
    }
}

struct Mesh::Impl
{
    PointContainer m_points;
    TriangleContainer m_triangles;
    QString m_name;

    std::unique_ptr<BoundingBox> mp_bbox_cache;
};


Mesh::Mesh()
    : mp_impl(std::make_unique<Impl>())
{
}

Mesh::Mesh(Mesh&& i_mesh)
    : Mesh()
{
    std::swap(mp_impl, i_mesh.mp_impl);
}

Mesh::~Mesh() = default;

MeshPoint* Mesh::AddPoint(const Point3D& i_point)
{
    if (auto p_point = GetPoint(i_point))
        return p_point;

    auto p_point = mp_impl->m_points.AddPoint(i_point);

    _InvalidateCache();
    return p_point;
}

MeshPoint* Mesh::AddPoint(double i_x, double i_y, double i_z)
{
    return AddPoint({ i_x, i_y, i_z });
}

TriangleHandle Mesh::AddTriangle(const Point3D& i_a, const Point3D& i_b, const Point3D& i_c)
{
    auto p_pnt1 = AddPoint(i_a);
    auto p_pnt2 = AddPoint(i_b);
    auto p_pnt3 = AddPoint(i_c);

    Q_ASSERT(p_pnt1);
    Q_ASSERT(p_pnt2);
    Q_ASSERT(p_pnt3);

    auto p_triangle = mp_impl->m_triangles.AddTriangle(Triangle{ *p_pnt1, *p_pnt2, *p_pnt3 });
    p_pnt1->AddTriangle(p_triangle);
    p_pnt2->AddTriangle(p_triangle);
    p_pnt3->AddTriangle(p_triangle);

    _InvalidateCache();

    return p_triangle;
}

MeshPoint* Mesh::GetPoint(const Point3D& i_point) const
{
    return mp_impl->m_points.GetPoint(i_point);
}

MeshPoint* Mesh::GetPoint(double i_x, double i_y, double i_z) const
{
    return GetPoint({ i_x, i_y, i_z });
}

MeshPoint* Mesh::GetPoint(size_t i_index) const
{
    Q_ASSERT(i_index >= 0 && i_index < GetPointsCount());
    return mp_impl->m_points.GetPointAt(i_index);
}

TriangleHandle Mesh::GetTriangleOrientationDependent(const Point3D& i_a, const Point3D& i_b, const Point3D& i_c) const
{
    return mp_impl->m_triangles.GetTriangle({ i_a, i_b, i_c });
}

TriangleHandle Mesh::GetTriangleOrientationIndependent(const Point3D& i_a, const Point3D& i_b, const Point3D& i_c) const
{
    if (auto p_tr = GetTriangleOrientationDependent(i_a, i_b, i_c).lock())
        return p_tr;
    return GetTriangleOrientationDependent(i_c, i_b, i_a);
}

TriangleHandle Mesh::GetTriangle(size_t i_index) const
{
    Q_ASSERT(i_index >= 0 && i_index < GetTrianglesCount());
    return mp_impl->m_triangles.GetTriangleAt(i_index);
}

TriangleHandle Mesh::GetTriangle(const Triangle& i_triangle) const
{
    return mp_impl->m_triangles.GetTriangle(i_triangle);
}

std::vector<TriangleHandle> Mesh::GetTrianglesIncidentToEdge(const Point3D& i_a, const Point3D& i_b) const
{
    return mp_impl->m_triangles.GetTrianglesIncidentToEdge(i_a, i_b);
}

std::vector<TriangleHandle> Mesh::GetTrianglesIncidentToPoint(const Point3D& i_point) const
{
    if (auto p_point = GetPoint(i_point))
    {
        auto& trs = p_point->GetTriangles();
        return std::vector<TriangleHandle>{ trs.begin(), trs.end() };
    }
    return std::vector<TriangleHandle>{};
}

void Mesh::UpdatePointCoordinates(const Point3D& i_old_coordinates, const Point3D& i_new_coordinates)
{
    MeshPoint* p_point = GetPoint(i_old_coordinates);
    if (!p_point)
    {
        Q_ASSERT(!"Can't find given point");
        return;
    }

    mp_impl->m_points.UpdatePointCoordinates(i_old_coordinates, i_new_coordinates);
    mp_impl->m_triangles.UpdateVertexPosition(i_old_coordinates, i_new_coordinates);

    _InvalidateCache();
}

void Mesh::RemovePoint(const Point3D& i_point)
{
    mp_impl->m_triangles.RemoveTrianglesWithVertex(i_point);
    mp_impl->m_points.RemovePoint(i_point);

    _InvalidateCache();
}

void Mesh::RemovePoint(double i_x, double i_y, double i_z)
{
    RemovePoint({ i_x, i_y, i_z });
}

void Mesh::RemoveTriangle(const Point3D& i_a, const Point3D& i_b, const Point3D& i_c)
{
    auto p_triangle = GetTriangleOrientationDependent(i_a, i_b, i_c );

    GetPoint(i_a)->RemoveTriangle(*p_triangle.lock());
    GetPoint(i_b)->RemoveTriangle(*p_triangle.lock());
    GetPoint(i_c)->RemoveTriangle(*p_triangle.lock());

    mp_impl->m_triangles.RemoveTriangle({ i_a, i_b, i_c });

    _InvalidateCache();
}

size_t Mesh::GetPointsCount() const
{
    return mp_impl->m_points.GetPointsCount();
}

size_t Mesh::GetTrianglesCount() const
{
    return mp_impl->m_triangles.GetTrianglesCount();
}

const QString& Mesh::GetName() const
{
    return mp_impl->m_name;
}

void Mesh::SetName(const QString& i_name) const
{
    mp_impl->m_name = i_name;
}

const BoundingBox& Mesh::GetBoundingBox() const
{
    if (!mp_impl->mp_bbox_cache)
    {
        mp_impl->mp_bbox_cache = std::make_unique<BoundingBox>();

        for (size_t i = 0; i < GetPointsCount(); ++i)
        {
            mp_impl->mp_bbox_cache->AddPoint(*GetPoint(i));
        }
    }

    return *mp_impl->mp_bbox_cache;
}

void Mesh::_InvalidateCache()
{
    mp_impl->mp_bbox_cache.reset();
}
