#include "Math.Algos/PointLocalizerVoxelized.h"

#include "Math.Algos/Voxelizer.h"

#include <Math.Core/CommonUtilities.h>
#include <Math.Core/Mesh.h>
#include <Math.Core/MeshTriangle.h>
#include <Math.Core/Point3D.h>
#include <Math.Core/TransformMatrix.h>
#include <Math.Core/Triangle.h>

#include <Math.DataStructures/VoxelGrid.h>

#include <list>
#include <limits>
#include <unordered_map>


namespace
{
    constexpr double DEFAULT_EPSILON = EPSILON;
}


struct PointLocalizerVoxelized::Impl 
{
    size_t m_next_mesh_index = 0;
    std::list<Triangle> m_transformed_triangles;
    std::unordered_map<Triangle*, size_t> m_triangles_to_mesh_map;
    std::shared_ptr<VoxelGrid> mp_voxelization;
};

PointLocalizerVoxelized::PointLocalizerVoxelized()
    : mp_impl(std::make_unique<Impl>())
{
}

PointLocalizerVoxelized::~PointLocalizerVoxelized()
{
}

size_t PointLocalizerVoxelized::AddMesh(const Mesh& i_mesh, const TransformMatrix& i_transformation)
{
    mp_impl->mp_voxelization.reset();

    const auto triangles_count = i_mesh.GetTrianglesCount();
    for (size_t i = 0; i < triangles_count; ++i)
    {
        if (auto p_triangle = i_mesh.GetTriangle(i).lock())
        {
            auto point1 = p_triangle->GetPoint(0);
            auto point2 = p_triangle->GetPoint(1);
            auto point3 = p_triangle->GetPoint(2);
            
            i_transformation.ApplyTransformation(point1);
            i_transformation.ApplyTransformation(point2);
            i_transformation.ApplyTransformation(point3);
        
            mp_impl->m_transformed_triangles.emplace_back(point1, point2, point3);
            mp_impl->m_triangles_to_mesh_map[&mp_impl->m_transformed_triangles.back()] = mp_impl->m_next_mesh_index;
        }
        else
        {
            assert(false);
        }
    }
   
    return mp_impl->m_next_mesh_index++;
}

void PointLocalizerVoxelized::Build(const Params& i_params)
{
    Voxelizer::Params params;
    params.m_resolution_x = i_params.m_voxel_size_x;
    params.m_resolution_y = i_params.m_voxel_size_y;
    params.m_resolution_z = i_params.m_voxel_size_z;
    params.m_precision = DEFAULT_EPSILON;

    Voxelizer voxelizer;
    voxelizer.SetParams(params);

    Mesh tmp_mesh;
    std::vector<Triangle*> triangles;
    triangles.reserve(mp_impl->m_transformed_triangles.size());
    std::transform(mp_impl->m_transformed_triangles.begin(), mp_impl->m_transformed_triangles.end(), std::back_inserter(triangles), [](auto& i_tr)
    {
        return &i_tr;
    });

    mp_impl->mp_voxelization = voxelizer.Voxelize(triangles, tmp_mesh);
}

size_t PointLocalizerVoxelized::Localize(const Point3D& i_point, ReturnCode* op_return_code)
{
    if (!mp_impl->mp_voxelization)
    {
        if (op_return_code)
            *op_return_code = ReturnCode::VoxelizationWasNotBuild;

        return std::numeric_limits<size_t>::max();
    }

    if (op_return_code)
        *op_return_code = ReturnCode::Ok;

    auto coordinates = mp_impl->mp_voxelization->GetCoordinatesForPoint(i_point);
    for (size_t x_coord = coordinates[0]; x_coord <= mp_impl->mp_voxelization->GetNumVoxels()[0]; ++x_coord)
    {
        const std::array<size_t, 3> current_coords = { x_coord, coordinates[1], coordinates[2] };
        if (auto p_voxel = mp_impl->mp_voxelization->GetVoxel(current_coords))
        {
            Triangle* p_nearest_triangle = nullptr;
            double distance = std::numeric_limits<double>::max();
            for (auto p_triangle : p_voxel->GetTriangles())
            {
                auto current_distance = Distance(i_point, *p_triangle);
                if (current_distance < distance)
                {
                    p_nearest_triangle = p_triangle;
                    distance = current_distance;
                }
            }

            if (p_nearest_triangle)
                return mp_impl->m_triangles_to_mesh_map[p_nearest_triangle];
        }
    }

    return std::numeric_limits<size_t>::max();
}

std::weak_ptr<VoxelGrid> PointLocalizerVoxelized::GetCachedGrid() const
{
    return mp_impl->mp_voxelization;
}
