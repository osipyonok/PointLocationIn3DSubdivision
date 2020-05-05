#pragma once

#include <Math.Algos/API.h>

#include <memory>
#include <vector>

class BoundingBox;
class Mesh;
class Triangle;
class VoxelGrid;

class MATH_ALGOS_API Voxelizer
{
public:
    struct Params
    {
        double m_resolution_x = 10.;
        double m_resolution_y = 10.;
        double m_resolution_z = 10.;
        double m_precision = 1e-7;
    };

    void SetParams(const Params& i_params);
    std::unique_ptr<VoxelGrid> Voxelize(const Mesh& i_mesh); 
    std::unique_ptr<VoxelGrid> Voxelize(const std::vector<Triangle*>& i_triangles);

private:
    Params m_params;
};
