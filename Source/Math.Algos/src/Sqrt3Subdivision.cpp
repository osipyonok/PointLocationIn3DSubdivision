#include "Math.Algos/Sqrt3Subdivision.h"

#include <Math.Core/CommonUtilities.h>
#include <Math.Core/Mesh.h>
#include <Math.Core/MeshPoint.h>
#include <Math.Core/MeshTriangle.h>
#include <Math.Core/Vector3D.h>
#include <Math.Core/VectorUtilities.h>


#include <map>
#include <vector>


namespace
{
    auto _AddTriangleKeepNormal(Mesh& io_mesh, const Triangle& i_triangle, const Vector3D& i_old_normal)
    {
        if (i_triangle.GetNormal() == i_old_normal)
        {
            return io_mesh.AddTriangle(i_triangle.GetPoint(0), i_triangle.GetPoint(1), i_triangle.GetPoint(2));
        }
        else
        {
            return io_mesh.AddTriangle(i_triangle.GetPoint(2), i_triangle.GetPoint(1), i_triangle.GetPoint(0));
        }
    }

    auto _GetMinEdgeLengthSqr(Triangle* ip_triangle)
    {
        auto edge1 = DistanceSqr(ip_triangle->GetPoint(0), ip_triangle->GetPoint(1));
        auto edge2 = DistanceSqr(ip_triangle->GetPoint(1), ip_triangle->GetPoint(2));
        auto edge3 = DistanceSqr(ip_triangle->GetPoint(2), ip_triangle->GetPoint(0));
        return std::min(edge1, std::min(edge2, edge3));
    }

    void _SubdivideFace(Mesh& i_mesh, const std::vector<MeshTriangle*>& i_triangles, SQRT3MeshSubdivider::Params i_params)
    {
        std::vector<MeshTriangle*> triangles = i_triangles;

        std::map<std::pair<const MeshPoint*, const MeshPoint*>, std::vector<MeshPoint*>> adjacent_centroids;
        auto make_edge = [](const MeshPoint* ip_left, const MeshPoint* ip_right)
        {
            return *ip_left < *ip_right ? std::make_pair(ip_left, ip_right) : std::make_pair(ip_right, ip_left);
        };

        while (!triangles.empty())
        {
            for (auto p_trinagle : triangles)
            {
                auto point1 = p_trinagle->GetPoint(0);
                auto point2 = p_trinagle->GetPoint(1);
                auto point3 = p_trinagle->GetPoint(2);

                auto centroid = (point1 + point2 + point3) / 3;
                auto p_centroid_mesh_point = i_mesh.AddPoint(centroid);

                auto old_normal = p_trinagle->GetNormal();

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

                auto old_normal = old_triangles[0]->GetNormal();
                Q_ASSERT(old_normal == old_triangles[1]->GetNormal());

                i_mesh.RemoveTriangle(old_triangles[0]->GetPoint(0), old_triangles[0]->GetPoint(1), old_triangles[0]->GetPoint(2));
                i_mesh.RemoveTriangle(old_triangles[1]->GetPoint(0), old_triangles[1]->GetPoint(1), old_triangles[1]->GetPoint(2));

                auto p_new_triangle1 = _AddTriangleKeepNormal(i_mesh, { centroid1, centroid2, shared_point1 }, old_normal);
                auto p_new_triangle2 = _AddTriangleKeepNormal(i_mesh, { centroid1, centroid2, shared_point2 }, old_normal);

                if (_GetMinEdgeLengthSqr(p_new_triangle1) > i_params.m_edge_length_threshold * i_params.m_edge_length_threshold)
                    triangles.emplace_back(p_new_triangle1);
                if (_GetMinEdgeLengthSqr(p_new_triangle2) > i_params.m_edge_length_threshold * i_params.m_edge_length_threshold)
                    triangles.emplace_back(p_new_triangle2);
            }
        }

    }
}


void SQRT3MeshSubdivider::SetParams(const Params& i_params)
{
    m_params = i_params;
}

void SQRT3MeshSubdivider::Subdivide(Mesh& i_mesh) const
{
    std::map<Point3D, std::vector<MeshTriangle*>> faces;

    for (size_t i = 0; i < i_mesh.GetTrianglesCount(); ++i)
    {
        auto p_triangle = i_mesh.GetTriangle(i);

        if (_GetMinEdgeLengthSqr(p_triangle) > m_params.m_edge_length_threshold * m_params.m_edge_length_threshold)
        {
            faces[p_triangle->GetNormal()].emplace_back(p_triangle);
        }
    }

    for(auto& face : faces)
    {
        _SubdivideFace(i_mesh, face.second, m_params);
    }
}
