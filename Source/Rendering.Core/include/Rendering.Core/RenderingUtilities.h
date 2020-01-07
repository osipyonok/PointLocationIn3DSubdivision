#pragma once

#include <Rendering.Core/API.h>

#include <memory>

namespace Qt3DRender
{
    class QGeometry;
}

class Mesh;

namespace Rendering
{
    namespace Utilities
    {
        RENDERING_CORE_API std::unique_ptr<Qt3DRender::QGeometry> Mesh2QGeometry(const Mesh& i_mesh);
    }
}