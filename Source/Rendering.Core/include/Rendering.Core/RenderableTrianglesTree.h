#pragma once

#include <Rendering.Core/API.h>

#include <Rendering.Core/IRenderable.h>

#include <Math.DataStructures/TrianglesTree.h>

#include <memory>

class QColor;

class TransformMatrix;

namespace Rendering
{
    class RENDERING_CORE_API RenderableTrianglesTree : public IRenderable
    {
        Q_OBJECT
            
    public:
        enum class RenderingStyle
        {
            Opaque,
            Transparent,
        };

        RenderableTrianglesTree(const TrianglesTree& i_tree);
        ~RenderableTrianglesTree() override;

        std::unique_ptr<Qt3DCore::QComponent> GetMaterial() const override;
        std::unique_ptr<Qt3DCore::QTransform> GetTransformation() const override;
        std::unique_ptr<Qt3DCore::QComponent> GetRenderer() const override;

        size_t GetLayersCount() const;
        size_t GetCurrentLayer() const;
        void SetCurrentLayer(size_t i_layer);

        void SetRenderingStyle(RenderingStyle i_style);
        RenderingStyle GetRenderingStyle() const;

        void SetColor(const QColor& i_color)  override;
        QColor GetColor() const override;
        void Transform(const TransformMatrix& i_transform) override;
        const TransformMatrix& GetTransform() const  override;

        std::vector<IRenderable*> GetNestedRenderables() const override;

    private:
        void _UpdateNestedRenderables();

        struct Impl;
        std::unique_ptr<Impl> mp_impl;
    };
}
