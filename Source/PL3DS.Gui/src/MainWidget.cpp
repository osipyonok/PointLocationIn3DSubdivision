#include "PL3DS.Gui/MainWidget.h"
#include "ui_MainWidget.h"

#include "PL3DS.Gui/MainWindow3D.h"
#include "PL3DS.Gui/RenderablesModel.h"

#include <Rendering.Core/IRenderable.h>
#include <Rendering.Core/RenderableBox.h>
#include <Rendering.Core/RenderableMesh.h>
#include <Rendering.Core/RenderablePoint.h>
#include <Rendering.Core/RenderableVoxelGrid.h>

#include <QAction>

#include <memory>


namespace
{
    void _InitConnections(const UI::MainWidget::Params& i_params, const Ui::MainWidget& i_ui, UI::MainWindow3D* ip_window_3d)
    {
        auto update_buttons_to_current_state = [=]
        {
            if (auto p_current_renderable = i_params.m_selected_renderable_getter())
            {
                i_ui.mp_btn_remove_selected->setEnabled(true);
                i_ui.mp_btn_toggle_visibility->setEnabled(true);
                i_ui.mp_btn_view_selected->setEnabled(true);

                auto p_model = i_ui.mp_list_renderables->model();
                auto indexes = p_model->match(p_model->index(0, 0), RenderablesModel::RawRenderablePtr, QVariant::fromValue(p_current_renderable), 1, Qt::MatchExactly);
                Q_ASSERT(indexes.size() == 1);
                const bool is_visible = indexes.at(0).data(RenderablesModel::Visibility).toBool();
                i_ui.mp_btn_toggle_visibility->setText(is_visible ? "Hide Selected" : "Show Selected");
                auto p_renderable = indexes.at(0).data(RenderablesModel::RawRenderablePtr).value<Rendering::IRenderable*>();
                const bool is_mesh = qobject_cast<Rendering::RenderableMesh*>(p_renderable);
                const bool is_bbox = qobject_cast<Rendering::RenderableBox*>(p_renderable);
                const bool is_grid = qobject_cast<Rendering::RenderableVoxelGrid*>(p_renderable);
                i_ui.mp_btn_transform->setEnabled(is_mesh);
                i_ui.mp_btn_subdivide->setEnabled(is_mesh);
                i_ui.mp_btn_toggle_view_mode->setEnabled(is_mesh || is_bbox || is_grid);
            }
            else
            {
                i_ui.mp_btn_remove_selected->setEnabled(false);
                i_ui.mp_btn_toggle_visibility->setEnabled(false);
                i_ui.mp_btn_transform->setEnabled(false);
                i_ui.mp_btn_subdivide->setEnabled(false);
                i_ui.mp_btn_view_selected->setEnabled(false);
                i_ui.mp_btn_toggle_view_mode->setEnabled(false);
            }
        };

        bool is_connected = false;
        is_connected = QObject::connect(i_ui.mp_btn_load_mesh, &QAbstractButton::clicked, i_params.mp_load_mesh_action, &QAction::trigger);
        Q_ASSERT(is_connected);

        is_connected = QObject::connect(i_ui.mp_btn_add_point, &QAbstractButton::clicked, i_params.mp_add_point_action, &QAction::trigger);
        Q_ASSERT(is_connected);

        is_connected = QObject::connect(i_ui.mp_btn_remove_selected, &QAbstractButton::clicked, i_params.mp_remove_selected_action, &QAction::trigger);
        Q_ASSERT(is_connected);

        is_connected = QObject::connect(i_ui.mp_btn_toggle_view_mode, &QAbstractButton::clicked, i_params.mp_toggle_view_mode_adction, &QAction::trigger);
        Q_ASSERT(is_connected);

        is_connected = QObject::connect(i_ui.mp_btn_toggle_visibility, &QAbstractButton::clicked, i_params.mp_toggle_visibility_action, [=]
        {
            i_params.mp_toggle_visibility_action->toggle();
            update_buttons_to_current_state();
        });
        Q_ASSERT(is_connected);

        is_connected = QObject::connect(i_ui.mp_btn_transform, &QAbstractButton::clicked, i_params.mp_translate_action, &QAction::trigger);
        Q_ASSERT(is_connected);

        is_connected = QObject::connect(i_ui.mp_btn_localize, &QAbstractButton::clicked, i_params.mp_localize_action, &QAction::trigger);
        Q_ASSERT(is_connected);

        is_connected = QObject::connect(i_ui.mp_btn_subdivide, &QAbstractButton::clicked, i_params.mp_subdivide_action, &QAction::trigger);
        Q_ASSERT(is_connected);

        is_connected = QObject::connect(i_ui.mp_list_renderables->selectionModel(), &QItemSelectionModel::selectionChanged, [=](const QItemSelection& i_selected, const QItemSelection& i_deselected)
        {
            if (!i_deselected.empty())
            {
                i_params.m_selected_renderable_setter(nullptr);
            }

            if (!i_selected.empty())
            {
                Q_ASSERT(i_selected.size() == 1);
                auto index = i_selected.indexes().at(0);
                i_params.m_selected_renderable_setter(index.data(RenderablesModel::RawRenderablePtr).value<Rendering::IRenderable*>());
            }

            update_buttons_to_current_state();
        });
        Q_ASSERT(is_connected);

        is_connected = QObject::connect(i_ui.mp_btn_view_all, &QAbstractButton::clicked, ip_window_3d, [=]
        {
            ip_window_3d->ViewAll();
        });
        Q_ASSERT(is_connected);

        is_connected = QObject::connect(i_ui.mp_btn_view_selected, &QAbstractButton::clicked, ip_window_3d, [=]
        {
            ip_window_3d->ViewRenderable(i_params.m_selected_renderable_getter());
        });
        Q_ASSERT(is_connected);

        is_connected = QObject::connect(ip_window_3d, &UI::MainWindow3D::ApproximatedPickedPoint, i_ui.mp_label_picked_point, [=](const Point3D& i_point)
        {
            auto text = QString("(%1, %2, %3)")
                       .arg(QString::number(i_point.GetX(), 'f'))
                       .arg(QString::number(i_point.GetY(), 'f'))
                       .arg(QString::number(i_point.GetZ(), 'f'));

            i_ui.mp_label_picked_point->setText(text);
        });
        Q_ASSERT(is_connected);

        update_buttons_to_current_state();

        Q_UNUSED(is_connected);
    }
}


namespace UI
{
    MainWidget::MainWidget(const Params& i_params, QWidget* ip_parent/* = nullptr*/)
        : QWidget(ip_parent)
    {
        Ui::MainWidget ui;
        ui.setupUi(this);

        auto p_window_3d = new UI::MainWindow3D;
        std::unique_ptr<QWidget> p_widget(QWidget::createWindowContainer(p_window_3d));

        ui.mp_main_widget->layout()->addWidget(p_widget.release());

        ui.mp_list_renderables->setModel(i_params.mp_renderables_model);

        _InitConnections(i_params, ui, p_window_3d);
    }


    MainWidget::~MainWidget() = default;

    void MainWidget::closeEvent(QCloseEvent* ip_event)
    {
        qApp->closeAllWindows();
    }

}
