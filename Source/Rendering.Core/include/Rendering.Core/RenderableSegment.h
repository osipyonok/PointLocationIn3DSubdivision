#pragma once

#include <Rendering.Core/API.h>

#include <Rendering.Core/IRenderable.h>

#include <Math.Core/Point3D.h>
#include <Math.Core/TransformMatrix.h>

#include <QColor>

namespace Rendering
{
    class RENDERING_CORE_API RenderableSegment : public Rendering::IRenderable
    {
        Q_OBJECT

    public:
        RenderableSegment(const Point3D& i_from, const Point3D& i_to);
        ~RenderableSegment() = default;

        std::unique_ptr<Qt3DCore::QComponent> GetMaterial() const;

        std::unique_ptr<Qt3DCore::QTransform> GetTransformation() const;

        std::unique_ptr<Qt3DCore::QComponent> GetRenderer() const;

        void SetColor(const QColor& i_color);
        QColor GetColor() const;
        void Transform(const TransformMatrix& i_transform);
        const TransformMatrix& GetTransform() const;

    private:
        Point3D m_from;
        Point3D m_to;
        TransformMatrix m_transform;
        QColor m_color;
    };
}
