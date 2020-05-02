#include "Rendering.Core/RenderableSegment.h"

#include "Rendering.Core/RenderingUtilities.h"

#include <Math.Core/CommonUtilities.h>
#include <Math.Core/Vector3D.h>
#include <Math.Core/VectorUtilities.h>

#include <Qt3DCore/QTransform>

#include <Qt3DExtras/QPhongMaterial>

#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QGeometry>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DExtras/QCylinderMesh>

#include <Klein/Render/ConcreteLinesRenderer.h>
#include <Klein/Render/UnlitMaterial.h>

#include <QtGlobal>

#include <cmath>

namespace
{
    constexpr float RADIUS = 0.001f;
    constexpr int SLICES = 20;
    constexpr int RINGS = 100;

    Vector3D _GetOrthogonalVector(const Vector3D& i_vector)
    {
        size_t non_zero_coord = 0;
        if (qFuzzyIsNull(i_vector.Get(non_zero_coord)))
            non_zero_coord = 1;
        if (qFuzzyIsNull(i_vector.Get(non_zero_coord)))
            non_zero_coord = 2;
        if(!qFuzzyIsNull(i_vector.Get(non_zero_coord)))
            return {};

        return Vector3D(
            non_zero_coord == 0 ? (i_vector.GetY() + i_vector.GetZ()) / i_vector.GetX() : -1,
            non_zero_coord == 1 ? (i_vector.GetX() + i_vector.GetZ()) / i_vector.GetY() : -1,
            non_zero_coord == 2 ? (i_vector.GetX() + i_vector.GetY()) / i_vector.GetZ() : -1
        );
    }

    using RotationMatrix = Matrix<double, 3>;
    RotationMatrix _RotateAtoB(const Vector3D& i_a, const Vector3D& i_b)
    {
        auto angle = Angle(i_a, i_b);
        auto cross = Cross(i_a, i_b);
        auto length = cross.Length();
        auto dist_to_pi = std::abs(PI - angle);
        auto dist_to_zero = std::abs(angle);
        if (!qFuzzyIsNull(length) || dist_to_pi < dist_to_zero)
        {
            auto axis = !qFuzzyIsNull(length) ? cross / length : _GetOrthogonalVector(i_a);
            const double mat_values[] = 
            {
                 0,           -axis.GetZ(),  axis.GetY(),
                 axis.GetZ(),  0,           -axis.GetX(),
                -axis.GetY(),  axis.GetX(),  0
            };

            RotationMatrix mat(mat_values);
            return RotationMatrix{} + std::sin(angle) * mat + (1 - std::cos(angle)) * mat * mat;
        }
        else // |angle| < eps -> return identity matrix
        {
            return RotationMatrix{};
        }
    }
}

namespace Rendering
{
    RenderableSegment::RenderableSegment(const Point3D& i_from, const Point3D& i_to)
        : m_from(i_from)
        , m_to(i_to)
    {
        static const auto original_vector = Vector3D(0, 1, 0);
        const auto expected_vector = Vector3D(i_to - i_from);
        const auto rotation = _RotateAtoB(original_vector, expected_vector);

        Point3D initial_from(0, -0.5, 0);
        Point3D initial_to(0,  0.5, 0);
        ::Transform(initial_from, rotation);
        ::Transform(initial_to, rotation);
        auto rotated_center = (initial_from + initial_to) / 2;
        auto expected_center = (i_from + i_to) / 2;
        m_transform.Translate(Vector3D(expected_center - rotated_center));

        TransformMatrix rotation_as_transform;
        for (size_t i = 0; i < 3; ++i)
            for (size_t j = 0; j < 3; ++j)
                rotation_as_transform.Item(i, j) = rotation(i, j);
        m_transform *= rotation_as_transform;
    }

    std::unique_ptr<Qt3DCore::QComponent> RenderableSegment::GetMaterial() const
    {
        auto p_material = std::make_unique<Klein::UnlitMaterial>();
        p_material->setBaseColor(m_color);
        return std::move(p_material);
    }

    std::unique_ptr<Qt3DCore::QTransform> RenderableSegment::GetTransformation() const
    {
        auto p_transform = std::make_unique<Qt3DCore::QTransform>();
        p_transform->setMatrix(Utilities::TransforMatrixToQMatrix4x4(m_transform));
        
        return std::move(p_transform);
    }

    std::unique_ptr<Qt3DCore::QComponent> RenderableSegment::GetRenderer() const
    {
        const QVector<QVector3D> positions = 
        { 
            {
                static_cast<float>(m_from.GetX()),
                static_cast<float>(m_from.GetY()),
                static_cast<float>(m_from.GetZ())
            }, 
            { 
                static_cast<float>(m_to.GetX()),
                static_cast<float>(m_to.GetY()),
                static_cast<float>(m_to.GetZ())
            } 
        };

        auto p_renderer = std::make_unique<Qt3DExtras::QCylinderMesh>();
        p_renderer->setRadius(RADIUS);
        p_renderer->setSlices(SLICES);
        p_renderer->setRings(RINGS);
        p_renderer->setLength(static_cast<float>(Distance(m_from, m_to)));
        return std::move(p_renderer);
    }

    void RenderableSegment::SetColor(const QColor& i_color)
    {
        if (m_color == i_color)
            return;

        m_color = i_color;
        emit RenderableMaterialChanged();
    }

    QColor RenderableSegment::GetColor() const
    {
        return m_color;
    }

    void RenderableSegment::Transform(const TransformMatrix& i_transform)
    {
        if (i_transform == TransformMatrix{})
            return;

        m_transform.PreMultiply(i_transform);

        emit RenderableTransformationChanged();
    }

    const TransformMatrix& RenderableSegment::GetTransform() const
    {
        return m_transform;
    }
}
