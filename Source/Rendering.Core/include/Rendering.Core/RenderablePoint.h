#pragma once

#include <Rendering.Core/API.h>

#include <Rendering.Core/IRenderable.h>

#include <memory>

class QColor;

class Point3D;

namespace Rendering
{
    class RENDERING_CORE_API RenderablePoint : public IRenderable
    {
        Q_OBJECT
    public:
        explicit RenderablePoint(const Point3D& i_point);
        ~RenderablePoint() override;

        std::unique_ptr<Qt3DCore::QComponent> GetMaterial() const override;
        std::unique_ptr<Qt3DCore::QTransform> GetTransformation() const override;
        std::unique_ptr<Qt3DCore::QComponent> GetRenderer() const override;

        void SetColor(const QColor& i_color) override;
        void Transform(const TransformMatrix& i_transform) override;
        
        QColor GetColor() const override;
        const TransformMatrix& GetTransform() const;

    private:
        struct Impl;
        std::unique_ptr<Impl> mp_impl;

    };
}

