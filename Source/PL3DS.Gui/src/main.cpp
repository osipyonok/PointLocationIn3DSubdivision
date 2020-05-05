#include <iostream>

#include <Math.Core/Triangle.h>
#include <Math.Core/Vector3D.h>
#include <Math.Core/Plane.h>
#include <Math.Core/Mesh.h>
#include <Math.Core/MeshPoint.h>
#include <Math.Core/MeshTriangle.h>

#include <Math.Algos/Sqrt3Subdivision.h>
#include <Math.Algos/Voxelizer.h>

#include <Math.DataStructures/TrianglesTree.h>

#include <Math.IO/MeshIO.h>

#include <Rendering.Core/RenderableBox.h>
#include <Rendering.Core/RenderableMesh.h>
#include <Rendering.Core/RenderablePoint.h>
#include <Rendering.Core/RenderableSegment.h>

#include <Rendering.Main/RenderablesController.h>
#include <Rendering.Main/RenderableFactory.h>

#include <QApplication>
#include <QDebug>
#include <QObject>
#include <QStyleFactory>
#include <QWidget>

#include "PL3DS.Gui/MainWidget.h"
#include "PL3DS.Gui/Actions/Actions.h"

#include "PL3DS.Gui/RenderablesModel.h"


namespace
{
    void _ConnectRenderablesModelToData(RenderablesModel& i_model, Rendering::RenderablesController& i_controller)
    {
        bool is_connected = false;

        is_connected = QObject::connect(&i_model, &QAbstractItemModel::dataChanged, &i_controller, [&](QModelIndex i_first, QModelIndex i_last, const QVector<int>& i_roles)
        {
            if (!i_roles.empty() && !i_roles.contains(RenderablesModel::Visibility) && !i_roles.contains(RenderablesModel::Color))
                return;

            for (int row = i_first.row(); row <= i_last.row(); ++row)
            {
                auto index = i_model.index(row);
                bool visible = i_model.data(index, RenderablesModel::Visibility).toBool();
                QColor color = i_model.data(index, RenderablesModel::Color).value<QColor>();
                auto p_renderable = i_model.data(index, RenderablesModel::RawRenderablePtr).value<Rendering::IRenderable*>();

                if (visible)
                {
                    i_controller.AddRenderable(*p_renderable);
                }
                else
                {
                    i_controller.RemoveRenderable(p_renderable);
                }

                p_renderable->SetColor(color);
            }
        });
        Q_ASSERT(is_connected);

        is_connected = QObject::connect(&i_model, &QAbstractItemModel::rowsInserted, &i_controller, [&](const QModelIndex&, int i_first, int i_last)
        {
            for (int row = i_first; row <= i_last; ++row)
            {
                auto index = i_model.index(row);
                if (index.data(RenderablesModel::Visibility).toBool())
                {
                    auto p_renderable = index.data(RenderablesModel::RawRenderablePtr).value<Rendering::IRenderable*>();
                    Rendering::RenderablesController::GetInstance().AddRenderable(*p_renderable);
                }
            }
        });
        Q_ASSERT(is_connected);

        is_connected = QObject::connect(&i_model, &QAbstractItemModel::rowsAboutToBeRemoved, &i_controller, [&](const QModelIndex&, int i_first, int i_last)
        {
            for (int row = i_first; row <= i_last; ++row)
            {
                auto index = i_model.index(row);
                if (index.data(RenderablesModel::Visibility).toBool())
                {
                    auto p_renderable = index.data(RenderablesModel::RawRenderablePtr).value<Rendering::IRenderable*>();
                    Rendering::RenderablesController::GetInstance().RemoveRenderable(p_renderable);
                }
            }
        });
        Q_ASSERT(is_connected);

        is_connected = QObject::connect(&i_model, &QAbstractItemModel::modelAboutToBeReset, &i_controller, [&]
        {
            Q_ASSERT(false);
        });
        Q_ASSERT(is_connected);

        Q_UNUSED(is_connected);
    }
}

void _AddCoordinateSystemMeshes()
{
    static constexpr double inf = 200'000;
    static Rendering::RenderableSegment axis_x({ -inf, 0, 0 }, { inf, 0, 0 });
    static Rendering::RenderableSegment axis_y({ 0, -inf, 0 }, { 0, inf, 0 });
    static Rendering::RenderableSegment axis_z({ 0, 0, -inf }, { 0, 0, inf });

    axis_x.SetColor(QColor("red"));
    axis_y.SetColor(QColor("green"));
    axis_z.SetColor(QColor("blue"));

    Rendering::RenderablesController::GetInstance().AddRenderable(axis_x);
    Rendering::RenderablesController::GetInstance().AddRenderable(axis_y);
    Rendering::RenderablesController::GetInstance().AddRenderable(axis_z);
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    if(auto p_style = QStyleFactory::create("Fusion"))
        QApplication::setStyle(p_style);
    
    _AddCoordinateSystemMeshes();

    _ConnectRenderablesModelToData(RenderablesModel::GetInstance(), Rendering::RenderablesController::GetInstance());

    UI::MainWidget::Params main_widget_params;
    main_widget_params.mp_renderables_model = &RenderablesModel::GetInstance();
    main_widget_params.m_selected_renderable_getter = &Actions::Configuration::GetCurrentRenderable;
    main_widget_params.m_selected_renderable_setter = &Actions::Configuration::SetCurrentRenderable;
    main_widget_params.mp_load_mesh_action = Actions::GetLoadMeshAction();
    main_widget_params.mp_add_point_action = Actions::GetAddPointAction();
    main_widget_params.mp_remove_selected_action = Actions::GetRemoveSelectedAction();
    main_widget_params.mp_voxelize_action = Actions::GetVoxelizeAction();
    main_widget_params.mp_toggle_visibility_action = Actions::GetHideShowSelectedAction();
    main_widget_params.mp_toggle_view_mode_adction = Actions::GetToggleViewModeAction();
    main_widget_params.mp_translate_action = Actions::GetTranslateAction();
    main_widget_params.mp_localize_action = Actions::GetLocalizeAction();
    main_widget_params.mp_subdivide_action = Actions::GetSubdivideAction();


    UI::MainWidget main_widget(main_widget_params);
    main_widget.showMaximized();


    if (argc != 1)
    {
        assert(argc == 2);
        Actions::RunLoadForMesh(argv[1]);
    }

    return app.exec();
}
