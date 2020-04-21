#pragma once

#include <Math.Algos/API.h>

class BoundingBox;
class Mesh;

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
    void Voxelize(const Mesh& i_mesh, Mesh& o_mesh);

private:
    Params m_params;
};
