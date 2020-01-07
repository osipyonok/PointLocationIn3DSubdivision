#include "Math.Core/TransformMatrix.h"

#include "Math.Core/Vector3D.h"

#include <QtMath>

#include <cmath>

void TransformMatrix::Rotate(double i_angle, const Vector3D& i_vector)
{
    Rotate(i_angle, i_vector.Get(0), i_vector.Get(1), i_vector.Get(2));
}

void TransformMatrix::Rotate(double i_angle, double i_x, double i_y, double i_z)
{
    if (i_angle == 0.)
        return;

    auto angle_rad = qDegreesToRadians(i_angle);
    auto sn = std::sin(angle_rad);    
    auto cs = std::cos(angle_rad);
    auto ics = 1. - cs;

    auto normalized = Vector3D(i_x, i_y, i_z).Normalized();

    auto x = normalized.GetX();
    auto y = normalized.GetY();
    auto z = normalized.GetZ();

    TransformMatrix rotation;

    rotation.Item(0, 0) = x * x * ics + cs;
    rotation.Item(0, 1) = x * y * ics - z * sn;
    rotation.Item(0, 2) = x * z * ics + y * sn;
    rotation.Item(0, 3) = 0.;

    rotation.Item(1, 0) = y * x * ics + z * sn;
    rotation.Item(1, 1) = y * y * ics + cs;
    rotation.Item(1, 2) = y * z * ics - x * sn;
    rotation.Item(1, 3) = 0.;

    rotation.Item(2, 0) = z * x * ics - y * sn;
    rotation.Item(2, 1) = z * y * ics + x * sn;
    rotation.Item(2, 2) = z * z * ics + cs;
    rotation.Item(2, 3) = 0.;

    rotation.Item(3, 0) = 0.;
    rotation.Item(3, 1) = 0.;
    rotation.Item(3, 2) = 0.;
    rotation.Item(3, 3) = 1.;

    *this *= rotation;
}

void TransformMatrix::Translate(const Vector3D& i_vector)
{
    Translate(i_vector.Get(0), i_vector.Get(1), i_vector.Get(2));
}

void TransformMatrix::Translate(double i_x, double i_y, double i_z)
{
    Item(0, 3) += Item(0, 0) * i_x + Item(0, 1) * i_y + Item(0, 2) * i_z;
    Item(1, 3) += Item(1, 0) * i_x + Item(1, 1) * i_y + Item(1, 2) * i_z;
    Item(2, 3) += Item(2, 0) * i_x + Item(2, 1) * i_y + Item(2, 2) * i_z;
    Item(3, 3) += Item(3, 0) * i_x + Item(3, 1) * i_y + Item(3, 2) * i_z;
}

void TransformMatrix::Scale(double i_factor)
{
    Scale(i_factor, i_factor, i_factor);
}

void TransformMatrix::Scale(double i_x, double i_y)
{
    Scale(i_x, i_y, 0.);
}

void TransformMatrix::Scale(double i_x, double i_y, double i_z)
{
    Item(0, 0) *= i_x;
    Item(1, 0) *= i_x;
    Item(2, 0) *= i_x;
    Item(3, 0) *= i_x;

    Item(0, 1) *= i_y;
    Item(1, 1) *= i_y;
    Item(2, 1) *= i_y;
    Item(3, 1) *= i_y;

    Item(0, 2) *= i_z;
    Item(1, 2) *= i_z;
    Item(2, 2) *= i_z;
    Item(3, 2) *= i_z;
}

void TransformMatrix::Scale(const Vector3D& i_vector)
{
    Scale(i_vector.Get(0), i_vector.Get(1), i_vector.Get(2));
}
