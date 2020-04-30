#pragma once

#include <QDialog>

#include <Math.Core/Point3D.h>

#include <Rendering.Core/IRenderable.h>

#include <memory>

class QString;

namespace UI
{
    namespace Details
    {
        class PointPreviewRenderable : public Rendering::IRenderable
        {
            Q_OBJECT
        public:
            PointPreviewRenderable();
            ~PointPreviewRenderable() override;

            std::unique_ptr<Qt3DCore::QComponent> GetMaterial() const override; 
            std::unique_ptr<Qt3DCore::QTransform> GetTransformation() const; 
            std::unique_ptr<Qt3DCore::QComponent> GetRenderer() const; 

            void SetColor(const QColor& i_color) override;
            QColor GetColor() const override;
            const TransformMatrix& GetTransform() const override; 
            void Transform(const TransformMatrix& i_transform) override;

            std::vector<Rendering::IRenderable*> GetNestedRenderables() const override;

            void SetPoint(const Point3D& i_point);

        private:
            Point3D m_position;
            std::vector<std::unique_ptr<Rendering::IRenderable>> m_renderables;
        };
    }

    class LocalizeDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit LocalizeDialog(QDialog* ip_parent = nullptr);
        ~LocalizeDialog() override;


    private:
        void _LogMessage(const QString& i_str) const;

        struct Impl;
        std::unique_ptr<Impl> mp_impl;
    };

}