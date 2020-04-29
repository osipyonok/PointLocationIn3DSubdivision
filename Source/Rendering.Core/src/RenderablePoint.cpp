#include "Rendering.Core/RenderablePoint.h"

#include "Rendering.Core/RenderingUtilities.h"

#include <Math.Core/Point3D.h>
#include <Math.Core/TransformMatrix.h>

#include <Klein/Render/UnlitMaterial.h>

#include <QColor>

#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QSphereMesh>

namespace
{
    constexpr float SPHERE_RADIUS = 0.01f;
}

struct Rendering::RenderablePoint::Impl 
{
    QColor m_color;
    Point3D m_point;
    TransformMatrix m_transform;
};

Rendering::RenderablePoint::RenderablePoint(const Point3D& i_point)
    : mp_impl(std::make_unique<Impl>())
{
    mp_impl->m_point = i_point;
    mp_impl->m_color = QColor("black");
}

Rendering::RenderablePoint::~RenderablePoint() = default;

std::unique_ptr<Qt3DCore::QComponent> Rendering::RenderablePoint::GetMaterial() const
{
    auto p_material = std::make_unique<Klein::UnlitMaterial>();
    p_material->setBaseColor(mp_impl->m_color);
    return std::move(p_material);
}

std::unique_ptr<Qt3DCore::QTransform> Rendering::RenderablePoint::GetTransformation() const
{
    auto p_transform = std::make_unique<Qt3DCore::QTransform>();
    auto tmp_matrix = mp_impl->m_transform;
    tmp_matrix.Translate(Vector3D{ mp_impl->m_point });
    p_transform->setMatrix(Rendering::Utilities::TransforMatrixToQMatrix4x4(tmp_matrix));

    return std::move(p_transform);
}

std::unique_ptr<Qt3DCore::QComponent> Rendering::RenderablePoint::GetRenderer() const
{
    auto p_rendered = std::make_unique<Qt3DExtras::QSphereMesh>();
    p_rendered->setRadius(SPHERE_RADIUS);

    return std::move(p_rendered);
}

void Rendering::RenderablePoint::SetColor(const QColor& i_color)
{
    if (mp_impl->m_color == i_color)
        return;

    mp_impl->m_color = i_color;
    emit RenderableMaterialChanged();
}

void Rendering::RenderablePoint::Transform(const TransformMatrix& i_transform)
{
    if (i_transform == TransformMatrix{})
        return;

    mp_impl->m_transform *= i_transform;
    emit RenderableTransformationChanged();
}

QColor Rendering::RenderablePoint::GetColor() const
{
    return mp_impl->m_color;
}

const TransformMatrix& Rendering::RenderablePoint::GetTransform() const
{
    return mp_impl->m_transform;
}
