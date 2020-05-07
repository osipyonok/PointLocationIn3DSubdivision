#include "Rendering.Core/RenderableVoxelGrid.h"

#include "Rendering.Core/RenderableMesh.h"

#include <Math.Core/Mesh.h>
#include <Math.Core/TransformMatrix.h>

#include <Math.DataStructures/VoxelGrid.h>

#include <Math.Algos/VoxelGrid2MeshConverter.h>

#include <QColor>

#include <Qt3DCore/QTransform>

#include <vector>

namespace Rendering
{
    struct RenderableVoxelGrid::Impl
    {
        std::unique_ptr<Mesh> mp_mesh;
        std::unique_ptr<RenderableMesh> mp_renderable_mesh;

        RenderingStyle m_style = RenderingStyle::Transparent;
    };

    RenderableVoxelGrid::RenderableVoxelGrid(const VoxelGrid& i_grid)
        : mp_impl(std::make_unique<Impl>())
    {
        mp_impl->mp_mesh = std::make_unique<Mesh>();
        VoxelGrid2MeshConverter::Convert(i_grid, *mp_impl->mp_mesh);

        mp_impl->mp_renderable_mesh = std::make_unique<RenderableMesh>(*mp_impl->mp_mesh);
        mp_impl->mp_renderable_mesh->SetColor(QColor(192, 192, 192)); // grey #C0C0C0
        mp_impl->mp_renderable_mesh->SetRenderingStyle(RenderableMesh::RenderingStyle::Transparent);

        bool is_connected = false;
        is_connected = connect(mp_impl->mp_renderable_mesh.get(), &IRenderable::RenderableMaterialChanged, this, &IRenderable::RenderableMaterialChanged);
        Q_ASSERT(is_connected);
        is_connected = connect(mp_impl->mp_renderable_mesh.get(), &IRenderable::RenderableTransformationChanged, this, &IRenderable::RenderableTransformationChanged);
        Q_ASSERT(is_connected);
        is_connected = connect(mp_impl->mp_renderable_mesh.get(), &IRenderable::RenderableRendererChanged, this, &IRenderable::RenderableRendererChanged);
        Q_ASSERT(is_connected);
        is_connected = connect(mp_impl->mp_renderable_mesh.get(), &IRenderable::RenderableDestructed, this, &IRenderable::RenderableDestructed);
        Q_ASSERT(is_connected);
        Q_UNUSED(is_connected);
    }

    RenderableVoxelGrid::~RenderableVoxelGrid() = default;

    std::unique_ptr<Qt3DCore::QComponent> RenderableVoxelGrid::GetMaterial() const
    {
        return mp_impl->mp_renderable_mesh->GetMaterial();
    }

    std::unique_ptr<Qt3DCore::QTransform> RenderableVoxelGrid::GetTransformation() const
    {
        return mp_impl->mp_renderable_mesh->GetTransformation();
    }

    std::unique_ptr<Qt3DCore::QComponent> RenderableVoxelGrid::GetRenderer() const
    {
        return mp_impl->mp_renderable_mesh->GetRenderer();
    }

    QColor RenderableVoxelGrid::GetColor() const
    {
        return mp_impl->mp_renderable_mesh->GetColor();
    }

    RenderableVoxelGrid::RenderingStyle RenderableVoxelGrid::GetRenderingStyle() const
    {
        return mp_impl->m_style;
    }

    const TransformMatrix& RenderableVoxelGrid::GetTransform() const
    {
        return mp_impl->mp_renderable_mesh->GetTransform();
    }

    void RenderableVoxelGrid::SetColor(const QColor& i_color)
    {
        mp_impl->mp_renderable_mesh->SetColor(i_color);
    }

    void RenderableVoxelGrid::SetRenderingStyle(RenderingStyle i_style)
    {
        mp_impl->m_style = i_style;

        switch (i_style)
        {
        case RenderingStyle::Opaque:
            mp_impl->mp_renderable_mesh->SetRenderingStyle(RenderableMesh::RenderingStyle::Opaque);
            break;
        case RenderingStyle::Transparent:
            mp_impl->mp_renderable_mesh->SetRenderingStyle(RenderableMesh::RenderingStyle::Transparent);
            break;
        default:
            Q_ASSERT(false);
        }
    }

    void RenderableVoxelGrid::Transform(const TransformMatrix& i_transform)
    {
        mp_impl->mp_renderable_mesh->Transform(i_transform);
    }

}
