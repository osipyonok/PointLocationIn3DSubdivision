#pragma once

#include <Math.Algos/API.h>

class Mesh;

class MATH_ALGOS_API SQRT3MeshSubdivider final
{
public:
    struct Params
    {
        bool m_apply_smoothing = true;
        double m_edge_length_threshold = 10;
    };


    void SetParams(const Params& i_params);
    void Subdivide(Mesh& i_mesh) const;

private:
    Params m_params;
};

