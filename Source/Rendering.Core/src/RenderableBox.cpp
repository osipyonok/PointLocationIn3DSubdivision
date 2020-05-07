#include "Rendering.Core/RenderableBox.h"

#include "Rendering.Core/RenderableMesh.h"
#include "Rendering.Core/RenderingUtilities.h"

#include <Math.Core/BoundingBox.h>
#include <Math.Core/Mesh.h>
#include <Math.Core/TransformMatrix.h>
#include <Math.Core/Vector3D.h>

#include <Klein/Render/UnlitSolidWireframeMaterial.h>
#include <Klein/Render/WBOITMaterial.h>

#include <QColor>

#include <Qt3DCore/QTransform>


namespace
{
    void _CreateBoxMesh(const BoundingBox& i_bbox, Mesh& io_mesh)
    {
        std::vector<Point3D> nodes;
        auto min_point = i_bbox.GetMin();
        auto max_point = i_bbox.GetMax();

        for (auto i = 0u; i < 8; ++i)
        {
            nodes.emplace_back(
                (i & 1) ? max_point.Get(0) : min_point.Get(0),
                (i & 2) ? max_point.Get(1) : min_point.Get(1),
                (i & 4) ? max_point.Get(2) : min_point.Get(2)
            );
        }

        static std::vector<std::tuple<int, int, int>> triangles = // xyz ordered indexes 
        {
            // bottom face
            {0, 1, 2},
            {1, 2, 3},
            // front face
            {0, 1, 5},
            {0, 5, 4},
            // left face
            {0, 2, 6},
            {0, 6, 4},
            // right face
            {1, 3, 7},
            {1, 7, 5},
            // back face
            {2, 3, 7},
            {2, 7, 6},
            // top face
            {4, 5, 7},
            {4, 7, 6}
        };

        for (auto tr : triangles)
        {
            auto i = std::get<0>(tr);
            auto j = std::get<1>(tr);
            auto k = std::get<2>(tr);
            io_mesh.AddTriangle(nodes[i], nodes[j], nodes[k]);
        }
    }
}

namespace Rendering
{
    struct RenderableBox::Impl 
    {
        BoundingBox m_bbox;
        RenderingStyle m_style = RenderingStyle::Opaque;
        std::unique_ptr<Mesh> mp_box_mesh;
        std::unique_ptr<RenderableMesh> mp_box_renderable_mesh;
    };

    RenderableBox::RenderableBox(const BoundingBox& i_bbox)
        : mp_impl(std::make_unique<Impl>())
    {
        mp_impl->m_bbox = i_bbox;
        mp_impl->mp_box_mesh = std::make_unique<Mesh>();
        _CreateBoxMesh(mp_impl->m_bbox, *mp_impl->mp_box_mesh);
        mp_impl->mp_box_renderable_mesh = std::make_unique<RenderableMesh>(*mp_impl->mp_box_mesh);
        mp_impl->mp_box_renderable_mesh->SetColor(QColor(192, 192, 192)); // grey #C0C0C0
        
        bool is_connected = false;
        is_connected = connect(mp_impl->mp_box_renderable_mesh.get(), &IRenderable::RenderableMaterialChanged, this, &IRenderable::RenderableMaterialChanged);
        Q_ASSERT(is_connected);
        is_connected = connect(mp_impl->mp_box_renderable_mesh.get(), &IRenderable::RenderableTransformationChanged, this, &IRenderable::RenderableTransformationChanged);
        Q_ASSERT(is_connected);
        is_connected = connect(mp_impl->mp_box_renderable_mesh.get(), &IRenderable::RenderableRendererChanged, this, &IRenderable::RenderableRendererChanged);
        Q_ASSERT(is_connected);
        is_connected = connect(mp_impl->mp_box_renderable_mesh.get(), &IRenderable::RenderableDestructed, this, &IRenderable::RenderableDestructed);
        Q_ASSERT(is_connected);
        Q_UNUSED(is_connected);
    }

    RenderableBox::~RenderableBox() = default;

    std::unique_ptr<Qt3DCore::QComponent> RenderableBox::GetMaterial() const
    {
        return mp_impl->mp_box_renderable_mesh->GetMaterial();
    }

    std::unique_ptr<Qt3DCore::QTransform> RenderableBox::GetTransformation() const
    {
        return mp_impl->mp_box_renderable_mesh->GetTransformation();
    }

    std::unique_ptr<Qt3DCore::QComponent> RenderableBox::GetRenderer() const
    {
        return mp_impl->mp_box_renderable_mesh->GetRenderer();
    }

    QColor RenderableBox::GetColor() const
    {
        return mp_impl->mp_box_renderable_mesh->GetColor();
    }

    RenderableBox::RenderingStyle RenderableBox::GetRenderingStyle() const
    {
        return mp_impl->m_style;
    }

    const TransformMatrix & RenderableBox::GetTransform() const
    {
        return mp_impl->mp_box_renderable_mesh->GetTransform();
    }

    void RenderableBox::SetColor(const QColor& i_color)
    {
        mp_impl->mp_box_renderable_mesh->SetColor(i_color);
    }

    void RenderableBox::SetRenderingStyle(RenderingStyle i_style)
    {
        switch (i_style)
        {
        case RenderingStyle::Opaque:
            mp_impl->mp_box_renderable_mesh->SetRenderingStyle(RenderableMesh::RenderingStyle::Opaque);
            break;
        case RenderingStyle::Transparent:
            mp_impl->mp_box_renderable_mesh->SetRenderingStyle(RenderableMesh::RenderingStyle::Transparent);
            break;
        default:
            Q_ASSERT(false);
        }
    }

    void RenderableBox::Transform(const TransformMatrix& i_transform)
    {
        mp_impl->mp_box_renderable_mesh->Transform(i_transform);
    }
}
