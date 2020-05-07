#include "Math.Algos/VoxelGrid2MeshConverter.h"

#include <Math.Core/CommonUtilities.h>
#include <Math.Core/Mesh.h>
#include <Math.Core/MeshPoint.h>
#include <Math.Core/MeshTriangle.h>
#include <Math.Core/Vector3D.h>

#include <Math.DataStructures/VoxelGrid.h>


namespace
{
    struct TriangleIndexes
    {
        size_t m_i = 0;
        size_t m_j = 0;
        size_t m_k = 0;
    };

    struct Quad
    {
        TriangleIndexes m_triangles[2];
    };

    inline Point3D _GetVertex(const BoundingBox& i_bbox, size_t i_index)
    {
        const auto min_point = i_bbox.GetMin();
        const auto max_point = i_bbox.GetMax();
        return Point3D(
            (i_index & 1) ? max_point.GetX() : min_point.GetX(),
            (i_index & 2) ? max_point.GetY() : min_point.GetY(),
            (i_index & 4) ? max_point.GetZ() : min_point.GetZ()
        );
    }

    std::vector<Quad> _GetDefaultRequiredQuads()
    {
        static std::unique_ptr<std::vector<Quad>> required_quads;
        if (!required_quads)
        {
            required_quads = std::make_unique<std::vector<Quad>>();

            Quad right_face;
            right_face.m_triangles[0] = { 1, 3, 7 };
            right_face.m_triangles[1] = { 1, 7, 5 };

            Quad back_face;
            back_face.m_triangles[0] = { 2, 3, 7 };
            back_face.m_triangles[1] = { 2, 7, 6 };

            Quad top_face;
            top_face.m_triangles[0] = { 4, 5, 7 };
            top_face.m_triangles[1] = { 4, 7, 6 };

            required_quads->emplace_back(right_face);
            required_quads->emplace_back(back_face);
            required_quads->emplace_back(top_face);
        }

        return *required_quads;
    }

    const std::vector<Quad>& _GetDefaultOptionalQuads()
    {
        static std::unique_ptr<std::vector<Quad>> optional_quads;
        if (!optional_quads)
        {
            optional_quads = std::make_unique<std::vector<Quad>>();

            Quad left_face;
            left_face.m_triangles[0] = { 0, 2, 6 };
            left_face.m_triangles[1] = { 0, 6, 4 };

            Quad front_face;
            front_face.m_triangles[0] = { 0, 1, 5 };
            front_face.m_triangles[1] = { 0, 5, 4 };

            Quad bottom_face;
            bottom_face.m_triangles[0] = { 0, 1, 2 };
            bottom_face.m_triangles[1] = { 1, 2, 3 };

            optional_quads->emplace_back(left_face);
            optional_quads->emplace_back(front_face);
            optional_quads->emplace_back(bottom_face);
        }

        return *optional_quads;
    }
}


namespace VoxelGrid2MeshConverter
{
    void Convert(const VoxelGrid& i_grid, Mesh& io_mesh)
    {
        const auto& num_voxels = i_grid.GetNumVoxels();
        const auto& voxel_size = i_grid.GetVoxelSize();

        const auto voxels = i_grid.GetExistingVoxels();

        for (const auto p_voxel : voxels)
        {
            auto voxel_coords = p_voxel->GetCoordinates();

            std::vector<Quad> required_quads = _GetDefaultRequiredQuads();
            
            for (size_t i = 0; i < 3; ++i)
            {
                auto prev_vox_coords = voxel_coords;
                if (voxel_coords[i] != 0)
                    --prev_vox_coords[i];

                bool was_voxel = voxel_coords[i] != 0 && i_grid.GetVoxel(prev_vox_coords);
                if (was_voxel)
                    continue;

                required_quads.emplace_back(_GetDefaultOptionalQuads()[i]);
            }

            for (const auto& quad : required_quads)
            {
                for (size_t i = 0; i < 2; ++i)
                {
                    io_mesh.AddTriangle(_GetVertex(*p_voxel, quad.m_triangles[i].m_i), 
                                        _GetVertex(*p_voxel, quad.m_triangles[i].m_j), 
                                        _GetVertex(*p_voxel, quad.m_triangles[i].m_k));
                }
            }

        }
    }
}
