#include "Math.Algos/Sqrt3Subdivision.h"

#include <Math.Core/CommonUtilities.h>
#include <Math.Core/Mesh.h>
#include <Math.Core/MeshPoint.h>
#include <Math.Core/MeshTriangle.h>
#include <Math.Core/Vector3D.h>
#include <Math.Core/VectorUtilities.h>

#include <boost/unordered_set.hpp>

#include <cassert>
#include <map>
#include <vector>


namespace
{
    inline auto _AddTriangleKeepNormal(Mesh& io_mesh, const Triangle& i_triangle, const Vector3D& i_old_normal)
    {
        if (i_triangle.GetNormal() == i_old_normal)
        {
            auto res = io_mesh.AddTriangle(i_triangle.GetPoint(0), i_triangle.GetPoint(1), i_triangle.GetPoint(2));

            assert(!io_mesh.GetPoint(i_triangle.GetPoint(0))->GetTriangles().empty());
            assert(!io_mesh.GetPoint(i_triangle.GetPoint(1))->GetTriangles().empty());
            assert(!io_mesh.GetPoint(i_triangle.GetPoint(2))->GetTriangles().empty());

            return res;
        }
        else
        {
            auto res = io_mesh.AddTriangle(i_triangle.GetPoint(2), i_triangle.GetPoint(1), i_triangle.GetPoint(0));

            assert(!io_mesh.GetPoint(i_triangle.GetPoint(0))->GetTriangles().empty());
            assert(!io_mesh.GetPoint(i_triangle.GetPoint(1))->GetTriangles().empty());
            assert(!io_mesh.GetPoint(i_triangle.GetPoint(2))->GetTriangles().empty());

            return res;
        }
    }

    inline auto _GetMinEdgeLengthSqr(Triangle* ip_triangle)
    {
        auto edge1 = DistanceSqr(ip_triangle->GetPoint(0), ip_triangle->GetPoint(1));
        auto edge2 = DistanceSqr(ip_triangle->GetPoint(1), ip_triangle->GetPoint(2));
        auto edge3 = DistanceSqr(ip_triangle->GetPoint(2), ip_triangle->GetPoint(0));
        return std::min(edge1, std::min(edge2, edge3));
    }

    void _SmoothOldPoints(Mesh& i_mesh, const boost::unordered_set<Point3D>& i_old_points)
    {
        std::vector<std::pair<Point3D, Point3D>> points_to_update;

        for (const auto& old_point : i_old_points)
        {
            auto p_mesh_point = i_mesh.GetPoint(old_point);
            auto neighbours = p_mesh_point->GetNeighbourPointsCount();
            double alpha = (4.0 - 2.0 * std::cos(2.0 * PI / static_cast<double>(neighbours))) / 9.0;

            Point3D average;
            for (const auto& neighbour : p_mesh_point->GetPoints())
                average += neighbour;
            average /= static_cast<double>(neighbours);
            
            points_to_update.emplace_back(old_point, (1.0 - alpha) * old_point + alpha * average);
        }

        for (const auto& point_to_update : points_to_update)
        {
            i_mesh.UpdatePointCoordinates(point_to_update.first, point_to_update.second);
        }
    }
}


void SQRT3MeshSubdivider::SetParams(const Params& i_params)
{
    m_params = i_params;
}

void SQRT3MeshSubdivider::Subdivide(Mesh& i_mesh) const
{
    std::vector<TriangleHandle> triangles;
    for (size_t i = 0; i < i_mesh.GetTrianglesCount(); ++i)
    {
        if (auto p_triangle = i_mesh.GetTriangle(i).lock())
        {
            if (_GetMinEdgeLengthSqr(p_triangle.get()) > m_params.m_edge_length_threshold * m_params.m_edge_length_threshold)
            {
                triangles.emplace_back(i_mesh.GetTriangle(i));
            }
        }
    }

    std::map<std::pair<const MeshPoint*, const MeshPoint*>, std::vector<MeshPoint*>> adjacent_centroids;
    auto make_edge = [](const MeshPoint* ip_left, const MeshPoint* ip_right)
    {
        return *ip_left < *ip_right ? std::make_pair(ip_left, ip_right) : std::make_pair(ip_right, ip_left);
    };

    while (!triangles.empty())
    {
        boost::unordered_set<Point3D> old_points;
        old_points.reserve(3 * triangles.size());

        for (auto p_trinagle : triangles)
        {
            auto point1 = p_trinagle.lock()->GetPoint(0);
            auto point2 = p_trinagle.lock()->GetPoint(1);
            auto point3 = p_trinagle.lock()->GetPoint(2);

            old_points.emplace(point1);
            old_points.emplace(point2);
            old_points.emplace(point3);

            auto centroid = (point1 + point2 + point3) / 3;
            auto p_centroid_mesh_point = i_mesh.AddPoint(centroid);

            auto old_normal = p_trinagle.lock()->GetNormal();

            i_mesh.RemoveTriangle(point1, point2, point3);

            auto p_new_triangle1 = _AddTriangleKeepNormal(i_mesh, { point1, point2, centroid }, old_normal);
            auto p_new_triangle2 = _AddTriangleKeepNormal(i_mesh, { point2, point3, centroid }, old_normal);
            auto p_new_triangle3 = _AddTriangleKeepNormal(i_mesh, { point3, point1, centroid }, old_normal);

            adjacent_centroids[make_edge(i_mesh.GetPoint(point1), i_mesh.GetPoint(point2))].push_back(p_centroid_mesh_point);
            adjacent_centroids[make_edge(i_mesh.GetPoint(point2), i_mesh.GetPoint(point3))].push_back(p_centroid_mesh_point);
            adjacent_centroids[make_edge(i_mesh.GetPoint(point3), i_mesh.GetPoint(point1))].push_back(p_centroid_mesh_point);
        }

        triangles.clear();
        for (auto old_edge_and_centroids : adjacent_centroids)
        {
            auto& old_edge = old_edge_and_centroids.first;
            auto& centroids = old_edge_and_centroids.second;

            if (centroids.size() != 2)
            {
                Q_ASSERT(centroids.size() == 1);
                continue;
            }

            auto old_triangles = i_mesh.GetTrianglesIncidentToEdge(*old_edge.first, *old_edge.second);
            if (old_triangles.size() != 2) // ?
                continue;

            Point3D shared_point1 = *old_edge.first;
            Point3D shared_point2 = *old_edge.second;

            Point3D centroid1 = *centroids[0];
            Point3D centroid2 = *centroids[1];

            auto old_normal = old_triangles[0].lock()->GetNormal();

            i_mesh.RemoveTriangle(old_triangles[0].lock()->GetPoint(0), old_triangles[0].lock()->GetPoint(1), old_triangles[0].lock()->GetPoint(2));
            i_mesh.RemoveTriangle(old_triangles[1].lock()->GetPoint(0), old_triangles[1].lock()->GetPoint(1), old_triangles[1].lock()->GetPoint(2));

            auto p_new_triangle1 = _AddTriangleKeepNormal(i_mesh, { centroid1, centroid2, shared_point1 }, old_normal).lock();
            auto p_new_triangle2 = _AddTriangleKeepNormal(i_mesh, { centroid1, centroid2, shared_point2 }, old_normal).lock();

            if (_GetMinEdgeLengthSqr(&*p_new_triangle1) > m_params.m_edge_length_threshold * m_params.m_edge_length_threshold)
                triangles.emplace_back(p_new_triangle1);
            if (_GetMinEdgeLengthSqr(&*p_new_triangle2) > m_params.m_edge_length_threshold * m_params.m_edge_length_threshold)
                triangles.emplace_back(p_new_triangle2);
        }

        if(m_params.m_apply_smoothing)
            _SmoothOldPoints(i_mesh, old_points);
    }

}
