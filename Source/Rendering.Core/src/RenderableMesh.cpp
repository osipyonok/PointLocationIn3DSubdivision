#include "Rendering.Core/RenderableMesh.h"

#include "Rendering.Core/RenderingUtilities.h"

#include <Math.Core/Mesh.h>

#include <Klein/Render/UnlitMaterial.h>
#include <Klein/Render/UnlitSolidWireframeMaterial.h>
#include <Klein/Render/WBOITMaterial.h>

#include <QMatrix4x4>

#include <Qt3DCore/QTransform>

#include <Qt3DRender/QMesh>

#include <Qt3DRender>


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

        QMatrix4x4 matrix_4x4;
        for (size_t i = 0; i < 4; ++i)
            for (size_t j = 0; j < 4; ++j)
                matrix_4x4(i, j) = m_transform(i, j);
        
        p_transform->setMatrix(matrix_4x4);    

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

    void RenderableMesh::SetColor(QColor i_color)
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

    void RenderableMesh::SetTransformMatrix(const TransformMatrix& i_transform)
    {
        if (i_transform == m_transform)
            return;

        m_transform = i_transform;
        emit RenderableTransformationChanged();
    }
}