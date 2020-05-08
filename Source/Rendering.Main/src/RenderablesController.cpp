#include "Rendering.Main/RenderablesController.h"

#include <Math.Core/Point3D.h>

#include <Rendering.Core/IRenderable.h>


namespace Rendering
{
    RenderablesController& RenderablesController::GetInstance()
    {
        static RenderablesController instance;
        return instance;
    }

    RenderablesController::RenderablesController()
    {
    }

    void RenderablesController::AddRenderable(IRenderable& i_renderable)
    {
        if (m_renderables.find(&i_renderable) != m_renderables.end())
            return;

        QVector<QMetaObject::Connection>& connections = m_renderables[&i_renderable];

        QMetaObject::Connection connection;
        connection = connect(&i_renderable, &IRenderable::RenderableMaterialChanged, this, [this, p_renderable = &i_renderable]
        {
            emit RenderableMaterialChanged(p_renderable);
        });
        connections.push_back(connection);
        Q_ASSERT(connection);

        connection = connect(&i_renderable, &IRenderable::RenderableRendererChanged, this, [this, p_renderable = &i_renderable]
        {
            emit RenderableRendererChanged(p_renderable);
        });
        connections.push_back(connection);
        Q_ASSERT(connection);

        connection = connect(&i_renderable, &IRenderable::RenderableTransformationChanged, this, [this, p_renderable = &i_renderable]
        {
            emit RenderableTransformationChanged(p_renderable);
        });
        connections.push_back(connection);
        Q_ASSERT(connection);

        connection = connect(&i_renderable, &IRenderable::RenderableDestructed, this, [this, p_renderable = &i_renderable]
        {
            RemoveRenderable(p_renderable);
        });
        connections.push_back(connection);
        Q_ASSERT(connection);

        connection = connect(&i_renderable, &IRenderable::NestedRenderablesAboutToBeReset, this, [this, p_renderable = &i_renderable]
        {
            emit NestedRenderablesAboutToBeReset(p_renderable);
        });
        connections.push_back(connection);
        Q_ASSERT(connection);

        connection = connect(&i_renderable, &IRenderable::NestedRenderablesReset, this, [this, p_renderable = &i_renderable]
        {
            emit NestedRenderablesReset(p_renderable);
        });
        connections.push_back(connection);
        Q_ASSERT(connection);

        emit RenderableAdded(&i_renderable);
    }

    void RenderablesController::RemoveRenderable(Rendering::IRenderable* ip_renderable)
    {
        if (!m_renderables.contains(ip_renderable))
            return;

        for (const auto& connection : m_renderables[ip_renderable])
            QObject::disconnect(connection);

        m_renderables.remove(ip_renderable);
        emit RenderableRemoved(ip_renderable);
    }

    std::list<IRenderable*> RenderablesController::GetRenderables()
    {
        std::list<IRenderable*> list;
        list.resize(m_renderables.size());
        std::copy(m_renderables.keyBegin(), m_renderables.keyEnd(), list.begin());
        return std::move(list);
    }

    BoundingBox RenderablesController::GetFitInBoundingBox() const
    {
        BoundingBox bbox;

        for (const auto p_renderable : m_renderables.keys())
        {
            auto renderable_bbox = p_renderable->GetBoundingBoxToFitInView();
            if (!renderable_bbox.IsValid())
                continue;

            bbox.AddPoint(renderable_bbox.GetMin());
            bbox.AddPoint(renderable_bbox.GetMax());
        }

        return bbox;
    }
}