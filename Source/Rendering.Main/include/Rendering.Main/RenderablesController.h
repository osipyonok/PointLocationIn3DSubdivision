#pragma once

#include <Rendering.Main/API.h>

#include <QSet>
#include <QObject>

#include <list>

namespace Rendering
{
    class IRenderable;

    class RENDERING_MAIN_API RenderablesController final : public QObject
    {
        Q_OBJECT

    public:
        static RenderablesController& GetInstance();

        void AddRenderable(IRenderable& i_renderable);
        void RemoveRenderable(Rendering::IRenderable* ip_renderable);

        std::list<IRenderable*> GetRenderables();

    Q_SIGNALS:
        void RenderableAdded(const Rendering::IRenderable*);
        void RenderableRemoved(const Rendering::IRenderable*);

        void RenderableMaterialChanged(const Rendering::IRenderable*);
        void RenderableTransformationChanged(const Rendering::IRenderable*);
        void RenderableRendererChanged(const Rendering::IRenderable*);

    private:
        RenderablesController();

        QSet<IRenderable*> m_renderables;
    };
}