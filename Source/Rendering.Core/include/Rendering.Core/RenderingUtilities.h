#pragma once

#include <Rendering.Core/API.h>

#include <memory>

namespace Qt3DRender
{
    class QGeometry;
}

class Mesh;
class TransformMatrix;

class QMatrix4x4;

namespace Rendering
{
    namespace Utilities
    {
        RENDERING_CORE_API std::unique_ptr<Qt3DRender::QGeometry> Mesh2QGeometry(const Mesh& i_mesh);
        RENDERING_CORE_API QMatrix4x4                             TransforMatrixToQMatrix4x4(const TransformMatrix& i_matrix);
    }
}