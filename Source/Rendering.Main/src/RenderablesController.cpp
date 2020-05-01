#include "Rendering.Main/RenderablesController.h"

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

        m_renderables.insert(&i_renderable);
        //todo disconnect on remove
        bool is_connected = connect(&i_renderable, &IRenderable::RenderableMaterialChanged, this, [this, p_renderable = &i_renderable]
        {
            emit RenderableMaterialChanged(p_renderable);
        });
        Q_ASSERT(is_connected);

        is_connected = connect(&i_renderable, &IRenderable::RenderableRendererChanged, this, [this, p_renderable = &i_renderable]
        {
            emit RenderableRendererChanged(p_renderable);
        });
        Q_ASSERT(is_connected);

        is_connected = connect(&i_renderable, &IRenderable::RenderableTransformationChanged, this, [this, p_renderable = &i_renderable]
        {
            emit RenderableTransformationChanged(p_renderable);
        });
        Q_ASSERT(is_connected);

        is_connected = connect(&i_renderable, &IRenderable::RenderableDestructed, this, [this, p_renderable = &i_renderable]
        {
            RemoveRenderable(p_renderable);
        });
        Q_ASSERT(is_connected);

        emit RenderableAdded(&i_renderable);

        Q_UNUSED(is_connected);
    }

    void RenderablesController::RemoveRenderable(Rendering::IRenderable* ip_renderable)
    {
        if (!m_renderables.contains(ip_renderable))
            return;

        m_renderables.remove(ip_renderable);
        emit RenderableRemoved(ip_renderable);
    }

    std::list<IRenderable*> RenderablesController::GetRenderables()
    {
        std::list<IRenderable*> list;
        list.resize(m_renderables.size());
        std::copy(m_renderables.begin(), m_renderables.end(), list.begin());
        return std::move(list);
    }
}