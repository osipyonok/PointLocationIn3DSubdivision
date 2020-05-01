#include "Rendering.Core/RenderableVoxelGrid.h"

#include "Rendering.Core/RenderableBox.h"

#include <Math.Core/TransformMatrix.h>

#include <Math.DataStructures/VoxelGrid.h>

#include <QColor>

#include <Qt3DCore/QTransform>

#include <vector>

namespace Rendering
{
    struct RenderableVoxelGrid::Impl
    {
        std::vector<RenderableBox*> mp_nested_renderables;
        RenderingStyle m_style = RenderingStyle::Transparent;
    };

    RenderableVoxelGrid::RenderableVoxelGrid(const VoxelGrid& i_grid)
        : mp_impl(std::make_unique<Impl>())
    {
        auto voxels = i_grid.GetExistingVoxels();
        mp_impl->mp_nested_renderables.reserve(voxels.size());
        for (const auto p_voxel : voxels)
        {
            auto p_voxel_renderable = std::make_unique<RenderableBox>(*p_voxel);
            p_voxel_renderable->setParent(this);
            p_voxel_renderable->SetRenderingStyle(RenderableBox::RenderingStyle::Transparent);
            mp_impl->mp_nested_renderables.emplace_back(p_voxel_renderable.release());
        }
    }

    RenderableVoxelGrid::~RenderableVoxelGrid()
    {
        _OnDestructed();
    }

    std::unique_ptr<Qt3DCore::QComponent> RenderableVoxelGrid::GetMaterial() const
    {
        return nullptr;
    }

    std::unique_ptr<Qt3DCore::QTransform> RenderableVoxelGrid::GetTransformation() const
    {
        return nullptr;
    }

    std::unique_ptr<Qt3DCore::QComponent> RenderableVoxelGrid::GetRenderer() const
    {
        return nullptr;
    }

    QColor RenderableVoxelGrid::GetColor() const
    {
        return mp_impl->mp_nested_renderables.empty() ? QColor{} : mp_impl->mp_nested_renderables.at(0)->GetColor();
    }

    RenderableVoxelGrid::RenderingStyle RenderableVoxelGrid::GetRenderingStyle() const
    {
        return mp_impl->m_style;
    }

    const TransformMatrix& RenderableVoxelGrid::GetTransform() const
    {
        return mp_impl->mp_nested_renderables.empty() ? TransformMatrix{} : mp_impl->mp_nested_renderables.at(0)->GetTransform();
    }

    void RenderableVoxelGrid::SetColor(const QColor& i_color)
    {
        if (i_color == GetColor())
            return;

        for (auto p_renderable : mp_impl->mp_nested_renderables)
            p_renderable->SetColor(i_color);

        emit RenderableMaterialChanged();
    }

    void RenderableVoxelGrid::SetRenderingStyle(RenderingStyle i_style)
    {
        if (i_style == mp_impl->m_style)
            return;

        mp_impl->m_style = i_style;
        auto new_style = i_style == RenderingStyle::Opaque ? RenderableBox::RenderingStyle::Opaque : RenderableBox::RenderingStyle::Transparent;
        for (auto p_renderable : mp_impl->mp_nested_renderables)
            p_renderable->SetRenderingStyle(new_style);

        emit RenderableMaterialChanged();
    }

    void RenderableVoxelGrid::Transform(const TransformMatrix& i_transform)
    {
        if (i_transform == TransformMatrix{})
            return;

        for (auto p_renderable : mp_impl->mp_nested_renderables)
            p_renderable->Transform(i_transform);

        emit RenderableTransformationChanged();
    }

    std::vector<IRenderable*> RenderableVoxelGrid::GetNestedRenderables() const
    {
        return std::vector<IRenderable*>(mp_impl->mp_nested_renderables.begin(), mp_impl->mp_nested_renderables.end());
    }

}
