#pragma once

#include <Math.Algos/API.h>

#include <memory>

class Point3D;
class Mesh;
class VoxelGrid;

class PointLocalizerVoxelized
{
    enum class ReturnCode
    {
        Ok,
        VoxelizationWasNotBuild,
        Error,
    };

    struct Params
    {
        double m_voxel_size_x = 1;
        double m_voxel_size_y = 1;
        double m_voxel_size_z = 1;
    };

    // returns mesh index
    size_t AddMesh(const Mesh& i_mesh);
    
    void Build(const Params& i_params);

    // returns index of mesh or std::numeric_limits<size_t>::max() if point is outside
    size_t Localize(const Point3D& i_point, ReturnCode* op_return_code = nullptr);

private:
#pragma warning(push)
#pragma warning(disable: 4251)
    struct Impl;
    std::unique_ptr<Impl> mp_impl;
#pragma warning(pop)
};
