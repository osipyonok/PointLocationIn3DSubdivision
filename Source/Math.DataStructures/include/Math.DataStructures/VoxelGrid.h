#pragma once

#include <Math.DataStructures/API.h>

#include <Math.Core/BoundingBox.h>

#include <array>
#include <vector>
#include <unordered_map>

class Point3D;
class Triangle;

class MATH_DATASTRUCTURES_API Voxel : public BoundingBox
{
public:
    Voxel(const std::array<size_t, 3>& i_coords, const Point3D& i_min_corner, const Point3D& i_max_corner);
    ~Voxel() = default;

    void AddTriangle(Triangle* ip_triangle);
    const std::vector<Triangle*>& GetTriangles() const;

    const std::array<size_t, 3>& GetCoordinates() const;

private:
    std::array<size_t, 3> m_coordinates;
#pragma warning(push)
#pragma warning(disable: 4251)
    std::vector<Triangle*> m_triangles;
#pragma warning(pop)
};

class MATH_DATASTRUCTURES_API VoxelGrid
{
public:
    VoxelGrid(const std::array<double, 3>& i_voxel_size, const std::array<size_t, 3>& i_num_voxels, const BoundingBox& i_bbox);

    const std::array<double, 3>& GetVoxelSize() const;
    const std::array<size_t, 3>& GetNumVoxels() const;
    Voxel* GetOrCreateVoxel(const std::array<size_t, 3>& i_coordinates);
    const Voxel* GetVoxel(const std::array<size_t, 3>& i_coordinates) const;
    std::vector<const Voxel*> GetExistingVoxels() const;
    bool PointInsideVoxelization(const Point3D& i_point) const;
    
    std::array<size_t, 3> GetCoordinatesForPoint(const Point3D& i_point) const;

    size_t GetVoxelIndexFromCoordinates(const std::array<size_t, 3>& i_coordinates) const;

private:
    BoundingBox m_bbox;
    std::array<double, 3> m_voxel_size;
    std::array<size_t, 3> m_num_voxels;
#pragma warning(push)
#pragma warning(disable: 4251)
    std::unordered_map<size_t, Voxel> m_voxels;
#pragma warning(pop)
};
