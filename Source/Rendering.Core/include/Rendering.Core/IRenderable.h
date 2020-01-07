#pragma once

#include <Rendering.Core/API.h>

#include <QObject>

#include <memory>

namespace Qt3DCore
{
	class QComponent;
	class QTransform;
}

namespace Rendering
{
	class RENDERING_CORE_API IRenderable : public QObject
	{
		Q_OBJECT
	public:
        ~IRenderable() override
        {
            Q_EMIT RenderableDestructed();
        }

		virtual std::unique_ptr<Qt3DCore::QComponent> GetMaterial() const = 0;

		virtual std::unique_ptr<Qt3DCore::QTransform> GetTransformation() const = 0;

		virtual std::unique_ptr<Qt3DCore::QComponent> GetRenderer() const = 0;


    Q_SIGNALS:
        void RenderableMaterialChanged();
        void RenderableTransformationChanged();
        void RenderableRendererChanged();

		void RenderableDestructed();
	};
}