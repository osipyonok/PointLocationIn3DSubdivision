#pragma once

#include <Rendering.Core/API.h>

#include <Rendering.Core/IRenderable.h>

#include <memory>

class QColor;

class BoundingBox;
class TransformMatrix;

namespace Rendering
{
    class RENDERING_CORE_API RenderableBox : public IRenderable
    {
        Q_OBJECT
    public:
        enum class RenderingStyle
        {
            Opaque,
            Transparent,
        };

        explicit RenderableBox(const BoundingBox& i_bbox);
        ~RenderableBox() override;

        std::unique_ptr<Qt3DCore::QComponent> GetMaterial() const override;
        std::unique_ptr<Qt3DCore::QTransform> GetTransformation() const override;
        std::unique_ptr<Qt3DCore::QComponent> GetRenderer() const override;

        QColor GetColor() const override;
        RenderingStyle GetRenderingStyle() const;
        const TransformMatrix& GetTransform() const;

        void SetColor(const QColor& i_color) override;
        void SetRenderingStyle(RenderingStyle i_style);
        void Transform(const TransformMatrix& i_transform) override;

    private:
        struct Impl;
        std::unique_ptr<Impl> mp_impl;
    };
}
