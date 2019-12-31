#pragma once

#include <Klein/Gui/AbstractQt3DWindow.h>

#include <memory>

namespace UI
{
	class MainWindow3D : public Klein::AbstractQt3DWindow
	{
		Q_OBJECT

	public:
		MainWindow3D(QWindow* ip_parent = nullptr);
		~MainWindow3D() override;

	protected:
		void resizeEvent(QResizeEvent*) override;

		Qt3DCore::QEntity* createSceneGraph() override;

		Qt3DRender::QRenderSettings* createRenderSettings(Qt3DCore::QEntity* root) override;

	private:
		struct Impl;
		std::unique_ptr<Impl> mp_impl;
	};
}