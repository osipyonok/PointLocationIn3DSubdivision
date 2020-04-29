#pragma once

#include <Rendering.Main/API.h>

#include <Rendering.Core/RenderableBox.h>
#include <Rendering.Core/RenderableMesh.h>
#include <Rendering.Core/RenderablePoint.h>
#include <Rendering.Core/RenderableVoxelGrid.h>

#include <memory>
#include <type_traits>

class BoundingBox;
class Mesh;
class Point3D;
class VoxelGrid;

namespace Rendering
{
    template<typename T>
    struct RenderableTypeFor;

    template<>
    struct RenderableTypeFor<BoundingBox> { using RenderableType = RenderableBox; };

    template<>
    struct RenderableTypeFor<Mesh> { using RenderableType = RenderableMesh; };

    template<>
    struct RenderableTypeFor<Point3D> { using RenderableType = RenderablePoint; };

    template<>
    struct RenderableTypeFor<VoxelGrid> { using RenderableType = RenderableVoxelGrid; };

    template<typename Object>
    auto CreateRenderableFor(Object&& i_renderable_object)
    {
        using ObjectWithoutCV = std::remove_cv_t<Object>;
        using ObjectWithoutRefAndCV = std::remove_reference_t<ObjectWithoutCV>;
        using RenderableType = typename RenderableTypeFor<ObjectWithoutRefAndCV>::RenderableType;
        return std::make_unique<RenderableType>(std::forward<Object>(i_renderable_object));
    }
}
