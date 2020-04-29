#include "Math.DataStructures/VoxelGrid.h"

#include <Math.Core/Point3D.h>

Voxel::Voxel(const std::array<size_t, 3>& i_coords, const Point3D& i_min_corner, const Point3D& i_max_corner)
    : BoundingBox()
    , m_coordinates(i_coords)
{
    AddPoint(i_min_corner);
    AddPoint(i_max_corner);
}

void Voxel::AddTriangle(Triangle* ip_triangle)
{
    m_triangles.emplace_back(ip_triangle);
}

const std::vector<Triangle*>& Voxel::GetTriangles() const
{
    return m_triangles;
}

const std::array<size_t, 3>& Voxel::GetCoordinates() const
{
    return m_coordinates;
}

////////////////////////////////////////////////////////////////

VoxelGrid::VoxelGrid(const std::array<double, 3>& i_voxel_size, const std::array<size_t, 3>& i_num_voxels, const BoundingBox& i_bbox)
    : m_voxel_size(i_voxel_size)
    , m_num_voxels(i_num_voxels)
    , m_bbox(i_bbox)
{
}

const std::array<double, 3>& VoxelGrid::GetVoxelSize() const
{
    return m_voxel_size;
}

Voxel* VoxelGrid::GetOrCreateVoxel(const std::array<size_t, 3>& i_coordinates)
{
    auto index = GetVoxelIndexFromCoordinates(i_coordinates);
    if (m_voxels.find(index) == m_voxels.end())
    {
        Point3D min_point(i_coordinates[0] * m_voxel_size[0], i_coordinates[1] * m_voxel_size[1], i_coordinates[2] * m_voxel_size[2]);
        Point3D max_point((i_coordinates[0] + 1) * m_voxel_size[0], (i_coordinates[1] + 1) * m_voxel_size[1], (i_coordinates[2] + 1) * m_voxel_size[2]);
        m_voxels.emplace(index, Voxel(i_coordinates, min_point + m_bbox.GetMin() , max_point + m_bbox.GetMin()));
    }
    return &m_voxels.at(index);
}

const Voxel* VoxelGrid::GetVoxel(const std::array<size_t, 3>& i_coordinates) const
{
    auto index = GetVoxelIndexFromCoordinates(i_coordinates);
    auto it = m_voxels.find(index);
    return it == m_voxels.end() ? nullptr : &it->second;
}

std::vector<const Voxel*> VoxelGrid::GetExistingVoxels() const
{
    std::vector<const Voxel*> result;
    result.reserve(m_voxels.size());
    std::transform(m_voxels.begin(), m_voxels.end(), std::back_inserter(result), [](const auto& i_value)
    {
        return &i_value.second;
    });
    std::sort(result.begin(), result.end(), [this](const auto p_voxel1, const auto p_voxel2)
    {
        return GetVoxelIndexFromCoordinates(p_voxel1->GetCoordinates())
             < GetVoxelIndexFromCoordinates(p_voxel2->GetCoordinates());
    });
    return std::move(result);
}

std::array<size_t, 3> VoxelGrid::GetCoordinatesForPoint(const Point3D& i_point) const
{
    auto min_point = m_bbox.GetMin();
    auto diff = i_point - min_point;

    return 
    {
        static_cast<size_t>(std::lroundl(diff.GetX() / m_voxel_size[0])),
        static_cast<size_t>(std::lroundl(diff.GetY() / m_voxel_size[1])),
        static_cast<size_t>(std::lroundl(diff.GetZ() / m_voxel_size[2]))
    };
}

size_t VoxelGrid::GetVoxelIndexFromCoordinates(const std::array<size_t, 3>& i_coordinates) const
{
    return i_coordinates[0] + i_coordinates[1] * m_num_voxels[0] + i_coordinates[2] * m_num_voxels[0] * m_num_voxels[1];
}
