#pragma once

#include <Math.Algos/API.h>

class Mesh;
class VoxelGrid;

namespace VoxelGrid2MeshConverter
{
    MATH_ALGOS_API void Convert(const VoxelGrid& i_grid, Mesh& io_mesh);
}
