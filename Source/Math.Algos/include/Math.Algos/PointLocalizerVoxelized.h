#pragma once

#include <Math.Algos/API.h>

#include <memory>

class Point3D;
class Mesh;
class TransformMatrix;
class VoxelGrid;

class PointLocalizerVoxelized
{
public:

    enum class ReturnCode
    {
        Ok,
        VoxelizationWasNotBuild,
    };

    struct Params
    {
        double m_voxel_size_x = 1;
        double m_voxel_size_y = 1;
        double m_voxel_size_z = 1;
    };

    MATH_ALGOS_API PointLocalizerVoxelized();
    MATH_ALGOS_API ~PointLocalizerVoxelized();

    // returns mesh index
    MATH_ALGOS_API size_t AddMesh(const Mesh& i_mesh, const TransformMatrix& i_transformation);
    
    MATH_ALGOS_API void Build(const Params& i_params);

    // returns index of mesh or std::numeric_limits<size_t>::max() if point is outside
    MATH_ALGOS_API size_t Localize(const Point3D& i_point, ReturnCode* op_return_code = nullptr);

private:
    struct Impl;
    std::unique_ptr<Impl> mp_impl;
};
