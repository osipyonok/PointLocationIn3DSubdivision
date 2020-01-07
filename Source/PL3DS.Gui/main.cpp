#include <iostream>

#include <Math.Core/Point3D.h>
#include <Math.Core/Triangle.h>
#include <Math.Core/Vector3D.h>
#include <Math.Core/Plane.h>
#include <Math.Core/Mesh.h>

#include <Rendering.Core/RenderableMesh.h>

#include <Rendering.Main/RenderablesController.h>

#include <QApplication>
#include <QWidget>

#include "MainWindow3D.h"


int main(int argc, char* argv[])
{
    QApplication app(argc, argv);


	/*          2
			   /|\
			  / | \
			 / /3\ \
			 0/___\ 1
	*/

    Mesh mesh;
    auto p_p0 = mesh.AddPoint({ 0, 0, 0 });
    auto p_p1 = mesh.AddPoint({ 1, 0, 0 });
    auto p_p2 = mesh.AddPoint({ 0, 0, 1 });
    auto p_p3 = mesh.AddPoint({ 0, 1, 0 });

    mesh.AddTriangle(*p_p0, *p_p1, *p_p2);// front
    mesh.AddTriangle(*p_p3, *p_p2, *p_p0);// left
    mesh.AddTriangle(*p_p1, *p_p2, *p_p3);// right
    mesh.AddTriangle(*p_p0, *p_p1, *p_p3);// bottom

    auto p_window = new UI::MainWindow3D;
    std::unique_ptr<QWidget> p_widget(QWidget::createWindowContainer(p_window));
    p_widget->show();

    auto p_renderable_mesh = std::make_unique<Rendering::RenderableMesh>(mesh);
    p_renderable_mesh->SetRenderingStyle(Rendering::RenderableMesh::RenderingStyle::Transparent);
    p_renderable_mesh->SetColor(QColor("teal"));
    Rendering::RenderablesController::GetInstance().AddRenderable(*p_renderable_mesh);

    Rendering::RenderableMesh renderable_mesh2(mesh);
    renderable_mesh2.SetRenderingStyle(Rendering::RenderableMesh::RenderingStyle::Opaque);
    renderable_mesh2.SetColor(QColor("grey"));
    Rendering::RenderablesController::GetInstance().AddRenderable(renderable_mesh2);
    TransformMatrix transform;
    transform.Translate({ -1, 0, -1 });
    transform.Scale(0.5);
    renderable_mesh2.SetTransformMatrix(transform);

	return app.exec();
}
