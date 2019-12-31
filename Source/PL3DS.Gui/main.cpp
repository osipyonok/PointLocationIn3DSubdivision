#include <iostream>

#include <Math.Core/Point3D.h>
#include <Math.Core/Triangle.h>
#include <Math.Core/Vector3D.h>
#include <Math.Core/Plane.h>

#include <QApplication>
#include <QWidget>

#include "MainWindow3D.h"


int main(int argc, char* argv[])
{
	//QGuiApplication app(argc, argv);
	QApplication app(argc, argv);


	/*          2
			   /|\
			  / | \
			 / /3\ \
			 0/___\ 1
	*/

	Point3D p0(0, 0, 0);
	Point3D p1(5, 0, 0);
	Point3D p2(0, 0, 1);
	Point3D p3(0, 1, 0);

	Triangle tr1(&p0, &p1, &p2); // front
	Triangle tr2(&p0, &p2, &p3); // left
	Triangle tr3(&p3, &p2, &p1); // right
	Triangle tr4(&p3, &p1, &p0); // bottom

	std::vector<Triangle*> trs = { &tr3, &tr4, &tr1, &tr2 };

	auto p_window = new UI::MainWindow3D;
	std::unique_ptr<QWidget> p_widget(QWidget::createWindowContainer(p_window));
	p_widget->show();
	//UI::MainWindow3D w;
	//w.showMaximized();


	return app.exec();
}

/*
int main(int argc, char** argv)
{
	std::cout << "Hello, master!\n";

	auto plane1 = Plane({ 1, 1, 1 }, { 1, 1, 1 });
	auto plane2 = Plane({ std::acos(-1), std::acos(-1), -3.283185307179586 }, { -17.8, -17.8, -17.8 });

	bool b1 = plane1 == plane2;
	bool b2 = plane2 == plane1;

	return 0;
}
*/