#pragma once

#include <Math.Core/API.h>

#include <Math.Core/Matrix.h>

#include <Math.Core/Vector3D.h>

class MATH_CORE_API TransformMatrix : public Matrix<double, 4>
{
public:
    using Matrix<double, 4>::Matrix;
    using Matrix<double, 4>::operator();
    using Matrix<double, 4>::operator*=;
    using Matrix<double, 4>::operator+=;
    using Matrix<double, 4>::operator/=;
    using Matrix<double, 4>::operator=;
    using Matrix<double, 4>::operator==;

    TransformMatrix();
    TransformMatrix(const double* ip_values);
    TransformMatrix(const Matrix<double, 4>& i_mat);
    
    // multiples this matrix by another that rotates coordinates through i_angle deg about i_vector
    void Rotate(double i_angle, const Vector3D& i_vector);

    // multiples this matrix by another that rotates coordinates through i_angle deg about vector (i_x, i_y, i_z)
    void Rotate(double i_angle, double i_x, double i_y, double i_z = 0);

    // translates by the components of i_vector
    void Translate(const Vector3D& i_vector);

    // translates by the components of vector (i_x, i_y, i_z)
    void Translate(double i_x, double i_y, double i_z = 0);

    // scales coordinates by the given factor
    void Scale(double i_factor);

    // scales coordinates by the components i_x and i_y
    void Scale(double i_x, double i_y);

    // scales coordinates by the components i_x, i_y and i_z 
    void Scale(double i_x, double i_y, double i_z);

    // scales coordinates by the components of i_vector
    void Scale(const Vector3D& i_vector);

    void ApplyTransformation(Point3D& io_point) const;
};