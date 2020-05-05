#include "Math.Algos/Voxelizer.h"


#include <Math.Core/BoundingBox.h>
#include <Math.Core/CommonUtilities.h>
#include <Math.Core/Mesh.h>
#include <Math.Core/MeshPoint.h>
#include <Math.Core/MeshTriangle.h>
#include <Math.Core/Point3D.h>

#include <Math.DataStructures/VoxelGrid.h>


void Voxelizer::SetParams(const Params& i_params)
{
    m_params = i_params;
}

std::unique_ptr<VoxelGrid> Voxelizer::Voxelize(const std::vector<Triangle*>& i_triangles)
{
    BoundingBox bbox;
    for (const auto& p_triangle : i_triangles)
    {
        bbox.AddPoint(p_triangle->GetPoint(0));
        bbox.AddPoint(p_triangle->GetPoint(1));
        bbox.AddPoint(p_triangle->GetPoint(2));
    }

    const auto cnt_x = static_cast<size_t>(std::max(1., std::ceil((bbox.GetDeltaX() + m_params.m_precision) / m_params.m_resolution_x)));
    const auto cnt_y = static_cast<size_t>(std::max(1., std::ceil((bbox.GetDeltaY() + m_params.m_precision) / m_params.m_resolution_y)));
    const auto cnt_z = static_cast<size_t>(std::max(1., std::ceil((bbox.GetDeltaZ() + m_params.m_precision) / m_params.m_resolution_z)));

    auto p_voxel_grid = std::make_unique<VoxelGrid>(std::array<double, 3>{ m_params.m_resolution_x, m_params.m_resolution_y, m_params.m_resolution_z },
                                                    std::array<size_t, 3>{ cnt_x, cnt_y, cnt_z },
                                                    bbox);

    std::vector<std::vector<std::vector<bool>>> is_present(cnt_x, std::vector<std::vector<bool>>(cnt_y, std::vector<bool>(cnt_z, false)));

    const Point3D step(m_params.m_resolution_x, m_params.m_resolution_y, m_params.m_resolution_z);

    for (const auto& p_triangle : i_triangles)
    {
        auto point1 = p_triangle->GetPoint(0);
        auto point2 = p_triangle->GetPoint(1);
        auto point3 = p_triangle->GetPoint(2);

        BoundingBox triangle_bbox;
        triangle_bbox.AddPoint(point1);
        triangle_bbox.AddPoint(point2);
        triangle_bbox.AddPoint(point3);

        auto bbox_min = bbox.GetMin();
        auto tria_min = triangle_bbox.GetMin();
        auto tria_max = triangle_bbox.GetMax();
        auto min_diff = tria_min - bbox_min;
        auto max_diff = tria_max - bbox_min;

        auto min_id_x = std::floorl(min_diff[0] / step[0]);
        auto min_id_y = std::floorl(min_diff[1] / step[1]);
        auto min_id_z = std::floorl(min_diff[2] / step[2]);

        if (qFuzzyCompare(min_id_x * m_params.m_resolution_x, min_diff[0]) && min_id_x > 0)
            --min_id_x;
        if (qFuzzyCompare(min_id_y * m_params.m_resolution_y, min_diff[1]) && min_id_y > 0)
            --min_id_y;
        if (qFuzzyCompare(min_id_z * m_params.m_resolution_z, min_diff[2]) && min_id_z > 0)
            --min_id_z;

        auto max_id_x = std::floorl(max_diff[0] / step[0]);
        auto max_id_y = std::floorl(max_diff[1] / step[1]);
        auto max_id_z = std::floorl(max_diff[2] / step[2]);

        if (qFuzzyCompare(max_id_x * m_params.m_resolution_x, max_diff[0]) && max_id_x < cnt_x)
            ++max_id_x;
        if (qFuzzyCompare(max_id_y * m_params.m_resolution_y, max_diff[1]) && max_id_y < cnt_y)
            ++max_id_y;
        if (qFuzzyCompare(max_id_z * m_params.m_resolution_z, max_diff[2]) && max_id_z < cnt_z)
            ++max_id_z;

        // probably can be done faster with bfs
        for (size_t x = min_id_x; x <= max_id_x; ++x)
        {
            for (size_t y = min_id_y; y <= max_id_y; ++y)
            {
                for (size_t z = min_id_z; z <= max_id_z; ++z)
                {
                    BoundingBox voxel;
                    voxel.AddPoint(Point3D(x * m_params.m_resolution_x, y * m_params.m_resolution_y, z * m_params.m_resolution_z) + bbox.GetMin());
                    voxel.AddPoint(Point3D((x + 1) * m_params.m_resolution_x, (y + 1) * m_params.m_resolution_y, (z + 1) * m_params.m_resolution_z) + bbox.GetMin());
                    ExtrudeInplace(voxel, m_params.m_precision);
                    if (TriangleWithBBoxIntersection(*p_triangle, voxel))
                    {
                        auto p_voxel = p_voxel_grid->GetOrCreateVoxel(std::array<size_t, 3>{ x, y, z });
                        p_voxel->AddTriangle(p_triangle);
                    }
                }
            }
        }
    }

    return std::move(p_voxel_grid);
}

std::unique_ptr<VoxelGrid> Voxelizer::Voxelize(const Mesh& i_mesh)
{
    std::vector<Triangle*> triangles;
    const auto triangles_count = i_mesh.GetTrianglesCount();
    triangles.reserve(triangles_count);
    for (size_t i = 0; i < triangles_count; ++i)
    {
        if (auto p_triangle = i_mesh.GetTriangle(i).lock())
        {
            triangles.emplace_back(p_triangle.get());
        }
        else
        {
            assert(false);
        }
    }

    return Voxelize(triangles);
}
