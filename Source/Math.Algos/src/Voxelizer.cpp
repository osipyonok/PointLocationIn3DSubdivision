#include "Math.Algos/Voxelizer.h"


#include <Math.Core/BoundingBox.h>
#include <Math.Core/CommonUtilities.h>
#include <Math.Core/Mesh.h>
#include <Math.Core/MeshPoint.h>
#include <Math.Core/MeshTriangle.h>
#include <Math.Core/Point3D.h>


void Voxelizer::SetParams(const Params& i_params)
{
    m_params = i_params;
}

void Voxelizer::Voxelize(const Mesh& i_mesh, Mesh& o_mesh)
{
    BoundingBox bbox;
    for (int i = 0; i < i_mesh.GetPointsCount(); ++i)
    {
        bbox.AddPoint(*i_mesh.GetPoint(i));
    }

    const auto cnt_x = static_cast<int>(std::max(1., std::ceil(bbox.GetDeltaX() / m_params.m_resolution_x))) + 1;
    const auto cnt_y = static_cast<int>(std::max(1., std::ceil(bbox.GetDeltaY() / m_params.m_resolution_y))) + 1;
    const auto cnt_z = static_cast<int>(std::max(1., std::ceil(bbox.GetDeltaZ() / m_params.m_resolution_z))) + 1;

    std::vector<std::vector<std::vector<bool>>> is_present(cnt_x, std::vector<std::vector<bool>>(cnt_y, std::vector<bool>(cnt_z, false)));

    const Point3D step(m_params.m_resolution_x, m_params.m_resolution_y, m_params.m_resolution_z);

    auto add_cube_if_not_exists = [&, this](int i_x, int i_y, int i_z)
    {
        if (is_present[i_x][i_y][i_z])
            return;
        is_present[i_x][i_y][i_z] = true;

        Point3D vertices[8] = 
        {
            *o_mesh.AddPoint(bbox.GetMin()[0] + i_x * step[0], bbox.GetMin()[1] + i_y * step[1], bbox.GetMin()[2] + i_z * step[2]),
            *o_mesh.AddPoint(bbox.GetMin()[0] + i_x * step[0], bbox.GetMin()[1] + (i_y + 1) * step[1], bbox.GetMin()[2] + i_z * step[2]),
            *o_mesh.AddPoint(bbox.GetMin()[0] + (i_x + 1) * step[0], bbox.GetMin()[1] + i_y * step[1], bbox.GetMin()[2] + i_z * step[2]),
            *o_mesh.AddPoint(bbox.GetMin()[0] + (i_x + 1) * step[0], bbox.GetMin()[1] + (i_y + 1) * step[1], bbox.GetMin()[2] + i_z * step[2]),
            *o_mesh.AddPoint(bbox.GetMin()[0] + i_x * step[0], bbox.GetMin()[1] + i_y * step[1], bbox.GetMin()[2] + (i_z + 1) * step[2]),
            *o_mesh.AddPoint(bbox.GetMin()[0] + i_x * step[0], bbox.GetMin()[1] + (i_y + 1) * step[1], bbox.GetMin()[2] + (i_z + 1) * step[2]),
            *o_mesh.AddPoint(bbox.GetMin()[0] + (i_x + 1) * step[0], bbox.GetMin()[1] + i_y * step[1], bbox.GetMin()[2] + (i_z + 1) * step[2]),
            *o_mesh.AddPoint(bbox.GetMin()[0] + (i_x + 1) * step[0], bbox.GetMin()[1] + (i_y + 1) * step[1], bbox.GetMin()[2] + (i_z + 1) * step[2]),
        };

        static std::vector<std::vector<int>> faces = 
        {
            { 3, 7, 2 },
            { 6, 2, 7},
            { 2, 6, 0},
            { 4, 0, 6},
            {0, 4, 1},
            {5, 1, 4},
            {1, 5, 3},
            {7, 3, 5},
            {3, 2, 0},
            {0, 1, 3},
            {7, 4, 6},
            {4, 7, 5}
        };
        
        for (auto& face : faces)
        {
            Triangle tr(vertices[face[0]], vertices[face[1]], vertices[face[2]]);
            if (auto p_triangle = o_mesh.GetTriangle(tr).lock())
                continue;

            o_mesh.AddTriangle(vertices[face[0]], vertices[face[1]], vertices[face[2]]);
        }

    };

    for (size_t i = 0; i < i_mesh.GetTrianglesCount(); ++i)
    {
        if (auto p_triangle = i_mesh.GetTriangle(i).lock())
        {
            auto point1 = p_triangle->GetPoint(0);
            auto point2 = p_triangle->GetPoint(1);
            auto point3 = p_triangle->GetPoint(2);

            BoundingBox triangle_bbox;
            triangle_bbox.AddPoint(point1);
            triangle_bbox.AddPoint(point2);
            triangle_bbox.AddPoint(point3);        

            auto min_id_x = (int)std::floor((triangle_bbox.GetMin() - bbox.GetMin())[0] / step[0]);
            auto min_id_y = (int)std::floor((triangle_bbox.GetMin() - bbox.GetMin())[1] / step[1]);
            auto min_id_z = (int)std::floor((triangle_bbox.GetMin() - bbox.GetMin())[2] / step[2]);

            auto max_id_x = (int)std::ceil((triangle_bbox.GetMax() - bbox.GetMin())[0] / step[0]);
            auto max_id_y = (int)std::ceil((triangle_bbox.GetMax() - bbox.GetMin())[1] / step[1]);
            auto max_id_z = (int)std::ceil((triangle_bbox.GetMax() - bbox.GetMin())[2] / step[2]);

            // probably can be done faster with bfs
            for (int x = min_id_x; x <= max_id_x; ++x)
            {
                for (int y = min_id_y; y <= max_id_y; ++y)
                {
                    for (int z = min_id_z; z <= max_id_z; ++z)
                    {
                        BoundingBox voxel;
                        voxel.AddPoint(Point3D(x * m_params.m_resolution_x, y * m_params.m_resolution_y, z * m_params.m_resolution_z) + bbox.GetMin());
                        voxel.AddPoint(Point3D((x + 1) * m_params.m_resolution_x, (y + 1) * m_params.m_resolution_y, (z + 1) * m_params.m_resolution_z) + bbox.GetMin());
                        ExtrudeInplace(voxel, m_params.m_precision);
                        if (TriangleWithBBoxIntersection(*p_triangle, voxel))
                        {
                            add_cube_if_not_exists(x, y, z);
                        }
                    }
                }
            }
        }
    }
}
