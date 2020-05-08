#pragma once

#include <Rendering.Core/API.h>

#include <Math.Core/BoundingBox.h>

#include <QObject>

#include <memory>
#include <vector>

class QColor;

class TransformMatrix;

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
            _OnDestructed();
        }

		virtual std::unique_ptr<Qt3DCore::QComponent> GetMaterial() const = 0;

		virtual std::unique_ptr<Qt3DCore::QTransform> GetTransformation() const = 0;

		virtual std::unique_ptr<Qt3DCore::QComponent> GetRenderer() const = 0;

        virtual void SetColor(const QColor& i_color) = 0;
        virtual QColor GetColor() const = 0;
        virtual void Transform(const TransformMatrix& i_transform) = 0;
        virtual const TransformMatrix& GetTransform() const = 0;

        virtual std::vector<IRenderable*> GetNestedRenderables() const { return {}; }
        virtual BoundingBox GetBoundingBoxToFitInView() const { return {}; };


    Q_SIGNALS:
        void RenderableMaterialChanged();
        void RenderableTransformationChanged();
        void RenderableRendererChanged();

        void NestedRenderablesAboutToBeReset();
        void NestedRenderablesReset();

		void RenderableDestructed();

    protected:
        // Should be called in destructor of your implementation in case you override GetNestedRenderables
        void _OnDestructed()
        {
            if (m_destructed)
                return;

            m_destructed = true;
            Q_EMIT RenderableDestructed();
        }

    private:
        bool m_destructed = false;
	};
}