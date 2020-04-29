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

#include "PL3DS.Gui/MainWindow3D.h"
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
    static Rendering::RenderableSegment axis_x({ 0, 0, 0 }, { 1, 0, 0 });
    static Rendering::RenderableSegment axis_y({ 0, 0, 0 }, { 0, 1, 0 });
    static Rendering::RenderableSegment axis_z({ 0, 0, 0 }, { 0, 0, 1 });

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

	/*          2
			   /|\
			  / | \
			 / /3\ \
			 0/___\ 1
	*/

    //Mesh mesh;

    //auto result = ReadMesh("C:/3dData/mesh/bunny.obj", mesh);
    //Q_ASSERT(result);


    /*
    auto point0 = mesh.AddPoint(0, 0, 0);
    auto point1 = mesh.AddPoint(2, 0, 0);
    auto point2 = mesh.AddPoint(2, 2, 0);
    auto point3 = mesh.AddPoint(0, 2, 0);
    auto point4 = mesh.AddPoint(1, 1, 0);

    mesh.AddTriangle(*point0, *point1, *point4);
    mesh.AddTriangle(*point1, *point2, *point4);
    mesh.AddTriangle(*point2, *point3, *point4);
    mesh.AddTriangle(*point3, *point0, *point4);
    */
    //mesh.UpdatePointCoordinates({ 1, 1, 0 }, { 1, 1, 1 });
    

    /*
    auto p_p0 = mesh.AddPoint({ 0, 0, 0 });
    auto p_p1 = mesh.AddPoint({ 1, 0, 0 });
    auto p_p2 = mesh.AddPoint({ 0, 0, 1 });
    auto p_p3 = mesh.AddPoint({ 0, 1, 0 });

    mesh.AddTriangle(*p_p0, *p_p1, *p_p2);// front
    mesh.AddTriangle(*p_p3, *p_p2, *p_p0);// left
    mesh.AddTriangle(*p_p1, *p_p2, *p_p3);// right
    mesh.AddTriangle(*p_p0, *p_p1, *p_p3);// bottom
    */


   //SQRT3MeshSubdivider subdivider;
   //subdivider.SetParams({ 0.1 });
   //subdivider.Subdivide(mesh);

    //Voxelizer voxelizer;
    //Voxelizer::Params params;
    //params.m_resolution_x = params.m_resolution_y = params.m_resolution_z = 0.3;
    //params.m_precision = 0.000003;
    //voxelizer.SetParams(params);

    //Mesh voxelized_mesh;
    //voxelizer.Voxelize(mesh, voxelized_mesh);

   
   /*for (size_t i = 0; i < mesh.GetTrianglesCount(); ++i)
   {
       auto tr = mesh.GetTriangle(i).lock();
       qDebug() << "Segment " << tr->GetPoint(0).GetX() << " " << tr->GetPoint(0).GetY()
                       << " " << tr->GetPoint(1).GetX() << " " << tr->GetPoint(1).GetY();
       qDebug() << "Segment " << tr->GetPoint(1).GetX() << " " << tr->GetPoint(1).GetY()
                       << " " << tr->GetPoint(2).GetX() << " " << tr->GetPoint(2).GetY();
       qDebug() << "Segment " << tr->GetPoint(2).GetX() << " " << tr->GetPoint(2).GetY()
                       << " " << tr->GetPoint(0).GetX() << " " << tr->GetPoint(0).GetY();
   }*/
   

    /*TrianglesTree kd_tree;
    
    std::vector<Triangle*> triangles;
    for (size_t i = 0; i < mesh.GetTrianglesCount(); ++i)
    {
        if (auto p_tr = mesh.GetTriangle(i).lock())
        {
            triangles.emplace_back(p_tr.get());
        }
    }
    kd_tree.Build(triangles);

    auto left_bbox = kd_tree.GetRoot().GetLeftChild().GetInfo().m_bbox;
    auto right_bbox = kd_tree.GetRoot().GetRightChild().GetInfo().m_bbox;*/
    
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


    //auto p_window = new MainWindow;
    //std::unique_ptr<QWidget> p_widget(QWidget::createWindowContainer(p_window));
    //p_widget->show();
    UI::MainWidget main_widget(main_widget_params);
    main_widget.showMaximized();

    if (argc != 1)
    {
        assert(argc == 2);
        Actions::RunLoadForMesh(argv[1]);
    }

    //auto p_renderable_mesh = std::make_unique<Rendering::RenderableMesh>(mesh);
    //auto p_renderable_mesh = Rendering::CreateRenderableFor(mesh);
    //p_renderable_mesh->SetRenderingStyle(Rendering::RenderableMesh::RenderingStyle::Transparent);
    //p_renderable_mesh->SetColor(QColor("teal"));
    //Rendering::RenderablesController::GetInstance().AddRenderable(*p_renderable_mesh);

    //RenderablesModel::GetInstance().AddRenderable(p_renderable_mesh.get(), "Bunny");

    //auto p_left_renderable = Rendering::CreateRenderableFor(left_bbox);
    //p_left_renderable->SetRenderingStyle(Rendering::RenderableBox::RenderingStyle::Transparent);
    //p_left_renderable->SetColor(QColor("yellow"));
    //Rendering::RenderablesController::GetInstance().AddRenderable(*p_left_renderable);

    //auto p_right_renderable = Rendering::CreateRenderableFor(right_bbox);
    //p_right_renderable->SetRenderingStyle(Rendering::RenderableBox::RenderingStyle::Transparent);
    //Rendering::RenderablesController::GetInstance().AddRenderable(*p_right_renderable);

    //Rendering::RenderableMesh renderable_mesh2(voxelized_mesh);
    //renderable_mesh2.SetRenderingStyle(Rendering::RenderableMesh::RenderingStyle::Transparent);
    //renderable_mesh2.SetColor(QColor("grey"));
    //Rendering::RenderablesController::GetInstance().AddRenderable(renderable_mesh2);
    //TransformMatrix transform;
    //transform.Translate({ -1, 0, -1 });
    //transform.Scale(0.5);
    //renderable_mesh2.SetTransformMatrix(transform);

	return app.exec();
}
