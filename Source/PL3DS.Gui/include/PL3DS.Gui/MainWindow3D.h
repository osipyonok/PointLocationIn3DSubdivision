#pragma once

#include <Klein/Gui/AbstractQt3DWindow.h>

#include <Math.Core/Point3D.h>

#include <memory>
#include <vector>

namespace Rendering { class IRenderable; }


namespace UI
{
	class MainWindow3D : public Klein::AbstractQt3DWindow
	{
		Q_OBJECT

	public:
		MainWindow3D(QWindow* ip_parent = nullptr);
		~MainWindow3D() override;

        void ViewAll();
        void ViewRenderable(const Rendering::IRenderable* ip_renderable);

    Q_SIGNALS:
        void ApproximatedPickedPoint(const Point3D&);

	protected:
		void resizeEvent(QResizeEvent*) override;

		Qt3DCore::QEntity* createSceneGraph() override;

		Qt3DRender::QRenderSettings* createRenderSettings(Qt3DCore::QEntity* root) override;

    private:
        void _CreateMeshFor(const Rendering::IRenderable* ip_renderable, Qt3DCore::QEntity* ip_parent) const;
        void _CreateMeshes(Qt3DCore::QEntity* ip_parent) const;

        Qt3DCore::QEntity* _GetRootEntity() const;

    private Q_SLOTS:
        void _OnRenderableAdded(const Rendering::IRenderable* ip_renderable);
        void _OnRenderableRemoved(const Rendering::IRenderable* ip_renderable);

        void _OnRenderableMaterialChanged(const Rendering::IRenderable* ip_renderable);
        void _OnRenderableTransformationChanged(const Rendering::IRenderable* ip_renderable);
        void _OnRenderableRendererChanged(const Rendering::IRenderable* ip_renderable);

        void _OnNestedRenderableAboutToBeReset(const Rendering::IRenderable* ip_renderable);
        void _OnNestedRenderableReset(const Rendering::IRenderable* ip_renderable);

	private:
		struct Impl;
		std::unique_ptr<Impl> mp_impl;
	};
}

Q_DECLARE_METATYPE(Point3D);
