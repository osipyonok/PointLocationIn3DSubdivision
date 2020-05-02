#include "Rendering.Core/RenderableMesh.h"

#include "Rendering.Core/RenderableSegment.h"
#include "Rendering.Core/RenderingUtilities.h"

#include <Math.Core/BoundingBox.h>
#include <Math.Core/Mesh.h>

#include <Klein/Render/UnlitMaterial.h>
#include <Klein/Render/UnlitSolidWireframeMaterial.h>
#include <Klein/Render/WBOITMaterial.h>

#include <QMatrix4x4>

#include <Qt3DCore/QTransform>

#include <Qt3DRender/QMesh>

#include <Qt3DRender>

#include <boost/scope_exit.hpp>


namespace Rendering
{
	RenderableMesh::RenderableMesh(Mesh& i_mesh)
        : mp_mesh(&i_mesh)
	{ 
        bool is_connected = QObject::connect(&i_mesh, &QObject::destroyed, this, &IRenderable::RenderableDestructed);
        Q_ASSERT(is_connected);
        Q_UNUSED(is_connected);
	}

    RenderableMesh::~RenderableMesh() = default;

    std::unique_ptr<Qt3DCore::QComponent> RenderableMesh::GetMaterial() const
    {
        switch (m_rendering_style)
        {
            case RenderingStyle::Opaque:
            {
                auto p_material = std::make_unique<Klein::UnlitSolidWireframeMaterial>();
                p_material->setBaseColor(m_color);
                return std::move(p_material);
            }
            case RenderingStyle::Transparent:
            {
                auto p_material = std::make_unique<Klein::WBOITMaterial>();
                p_material->setBaseColor(m_color);
                return std::move(p_material);
            }
            default:
                Q_ASSERT(false);
                break;
        }

        return nullptr;
    }

    std::unique_ptr<Qt3DCore::QTransform> RenderableMesh::GetTransformation() const
    {
        auto p_transform = std::make_unique<Qt3DCore::QTransform>();    
        p_transform->setMatrix(Rendering::Utilities::TransforMatrixToQMatrix4x4(m_transform));    

        return std::move(p_transform);
    }

    std::unique_ptr<Qt3DCore::QComponent> RenderableMesh::GetRenderer() const
    {
        if (!mp_mesh)
            return nullptr;

        auto p_renderer = std::make_unique<Qt3DRender::QGeometryRenderer>();
        auto p_geomerty = Utilities::Mesh2QGeometry(*mp_mesh);

        p_geomerty->setParent(p_renderer.get());

        p_renderer->setInstanceCount(1);
        p_renderer->setFirstVertex(0);
        p_renderer->setFirstInstance(0);
        p_renderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Triangles);
        p_renderer->setVertexCount(static_cast<int>(3 * mp_mesh->GetTrianglesCount()));
        p_renderer->setGeometry(p_geomerty.release());

        return std::move(p_renderer);
    }

    Mesh* RenderableMesh::GetMesh() const
    {
        return mp_mesh.data();
    }

    QColor RenderableMesh::GetColor() const
    {
        return m_color;
    }

    RenderableMesh::RenderingStyle RenderableMesh::GetRenderingStyle() const
    {
        return m_rendering_style;
    }

    const TransformMatrix& RenderableMesh::GetTransform() const
    {
        return m_transform;
    }

    void RenderableMesh::SetColor(const QColor& i_color)
    {
        if (m_color == i_color)
            return;

        m_color = i_color;
        emit RenderableMaterialChanged();
    }

    void RenderableMesh::SetRenderingStyle(RenderingStyle i_style)
    {
        if (m_rendering_style == i_style)
            return;

        m_rendering_style = i_style;
        emit RenderableMaterialChanged();
    }

    void RenderableMesh::Transform(const TransformMatrix& i_transform)
    {
        if (i_transform == TransformMatrix{})
            return;

        m_transform.PreMultiply(i_transform);

        for (const auto& p_nested_renderable : m_bbox_contours)
        {
            p_nested_renderable->Transform(i_transform);
        }

        emit RenderableTransformationChanged();
    }

    std::vector<IRenderable*> RenderableMesh::GetNestedRenderables() const
    {
        std::vector<IRenderable*> renderables;
        renderables.reserve(m_bbox_contours.size());

        for (auto& p_renderable : m_bbox_contours)
            renderables.emplace_back(p_renderable.get());

        return std::move(renderables);
    }

    void RenderableMesh::SetDrawBoundingBoxContours(bool i_draw)
    {
        if (i_draw == m_draw_bbox_contours)
            return;

        m_draw_bbox_contours = i_draw;

        emit NestedRenderablesAboutToBeReset();

        BOOST_SCOPE_EXIT(this_)
        {
            emit this_->NestedRenderablesReset();
        }
        BOOST_SCOPE_EXIT_END

        if (!m_draw_bbox_contours)
        {
            m_bbox_contours.clear();
            return;
        }

        const auto& bbox = mp_mesh->GetBoundingBox();
        auto bbox_min = bbox.GetMin();
        auto bbox_max = bbox.GetMax();

        //zyx ordered
        Point3D bottom_plane[4] = 
        {
            bbox_min,
            Point3D(bbox_max.GetX(), bbox_min.GetY(), bbox_min.GetZ()),
            Point3D(bbox_min.GetX(), bbox_max.GetY(), bbox_min.GetZ()),
            Point3D(bbox_max.GetX(), bbox_max.GetY(), bbox_min.GetZ())
        };
        Point3D top_plane[4] =
        {
            Point3D(bbox_min.GetX(), bbox_min.GetY(), bbox_max.GetZ()),
            Point3D(bbox_max.GetX(), bbox_min.GetY(), bbox_max.GetZ()),
            Point3D(bbox_min.GetX(), bbox_max.GetY(), bbox_max.GetZ()),
            bbox_max
        };

        //bottom plane
        m_bbox_contours.emplace_back(std::make_unique<RenderableSegment>(bottom_plane[0], bottom_plane[1]));
        m_bbox_contours.emplace_back(std::make_unique<RenderableSegment>(bottom_plane[0], bottom_plane[2]));
        m_bbox_contours.emplace_back(std::make_unique<RenderableSegment>(bottom_plane[1], bottom_plane[3]));
        m_bbox_contours.emplace_back(std::make_unique<RenderableSegment>(bottom_plane[2], bottom_plane[3]));

        //top plane
        m_bbox_contours.emplace_back(std::make_unique<RenderableSegment>(top_plane[0], top_plane[1]));
        m_bbox_contours.emplace_back(std::make_unique<RenderableSegment>(top_plane[0], top_plane[2]));
        m_bbox_contours.emplace_back(std::make_unique<RenderableSegment>(top_plane[1], top_plane[3]));
        m_bbox_contours.emplace_back(std::make_unique<RenderableSegment>(top_plane[2], top_plane[3]));

        //front face
        m_bbox_contours.emplace_back(std::make_unique<RenderableSegment>(bottom_plane[0], top_plane[0]));
        m_bbox_contours.emplace_back(std::make_unique<RenderableSegment>(bottom_plane[1], top_plane[1]));

        //back face
        m_bbox_contours.emplace_back(std::make_unique<RenderableSegment>(bottom_plane[2], top_plane[2]));
        m_bbox_contours.emplace_back(std::make_unique<RenderableSegment>(bottom_plane[3], top_plane[3]));

        for (const auto& p_nested_renderable : m_bbox_contours)
        {
            p_nested_renderable->SetColor(QColor("orange"));
            p_nested_renderable->Transform(m_transform);
        }
    }
}
