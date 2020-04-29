#include "Math.Core/VectorUtilities.h"

#include "Math.Core/Vector3D.h"

double Angle(const Vector3D& i_vec1, const Vector3D& i_vec2)
{
	return std::acos((Dot(i_vec1, i_vec2) / i_vec1.Length()) / i_vec2.Length());
}

Vector3D Cross(const Vector3D& i_vec1, const Vector3D& i_vec2)
{
	auto x = i_vec1.GetY() * i_vec2.GetZ() - i_vec1.GetZ() * i_vec2.GetY();
	auto y = i_vec1.GetZ() * i_vec2.GetX() - i_vec1.GetX() * i_vec2.GetZ();
	auto z = i_vec1.GetX() * i_vec2.GetY() - i_vec1.GetY() * i_vec2.GetX();
	return { x, y, z };
}

double Dot(const Vector3D& i_vec1, const Vector3D& i_vec2)
{
	return i_vec1.Get(0) * i_vec2.Get(0) + i_vec1.Get(1) * i_vec2.Get(1) + i_vec1.Get(2) * i_vec2.Get(2);
}

void Transform(Point3D& io_vector, const Matrix<double, 3>& i_mat)
{
    auto x = i_mat.Item(0, 0) * io_vector.Get(0) + i_mat.Item(0, 1) * io_vector.Get(1) + i_mat.Item(0, 2) * io_vector.Get(2);
    auto y = i_mat.Item(1, 0) * io_vector.Get(0) + i_mat.Item(1, 1) * io_vector.Get(1) + i_mat.Item(1, 2) * io_vector.Get(2);
    auto z = i_mat.Item(1, 0) * io_vector.Get(0) + i_mat.Item(2, 1) * io_vector.Get(1) + i_mat.Item(2, 2) * io_vector.Get(2);
    io_vector.SetX(x);
    io_vector.SetY(y);
    io_vector.SetZ(z);
}

