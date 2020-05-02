#pragma once

#include <Rendering.Core/API.h>

#include <Rendering.Core/IRenderable.h>

#include <Math.Core/TransformMatrix.h>

#include <QColor>
#include <QPointer>

class Mesh;

namespace Rendering
{
	class RENDERING_CORE_API RenderableMesh : public IRenderable
	{
		Q_OBJECT
	public:
        enum class RenderingStyle
        {
            Opaque,
            Transparent,
        };

		explicit RenderableMesh(Mesh& i_mesh);
		~RenderableMesh() override;

		std::unique_ptr<Qt3DCore::QComponent> GetMaterial() const override;
		std::unique_ptr<Qt3DCore::QTransform> GetTransformation() const override;
		std::unique_ptr<Qt3DCore::QComponent> GetRenderer() const override;

        Mesh* GetMesh() const;

        QColor GetColor() const override;
        RenderingStyle GetRenderingStyle() const;
        const TransformMatrix& GetTransform() const;

        void SetColor(const QColor& i_color) override;
        void SetRenderingStyle(RenderingStyle i_style);
        void Transform(const TransformMatrix& i_transform);

        void SetDrawBoundingBoxContours(bool i_draw);

        std::vector<IRenderable*> GetNestedRenderables() const override;

	private:
		QPointer<Mesh> mp_mesh;

        QColor m_color = QColor(0, 0, 0);
        TransformMatrix m_transform;
        RenderingStyle m_rendering_style = RenderingStyle::Opaque;

        bool m_draw_bbox_contours = false;
        std::vector<std::unique_ptr<Rendering::IRenderable>> m_bbox_contours;
	};
}