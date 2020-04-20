#pragma once

#include <Math.Core/API.h>

#include <Math.Core/Triangle.h>

#include <array>
#include <memory>

#include <boost/optional.hpp>

class Point3D;

/*
  ----b-------
  | A /\  B  /
  |  /  \   /
  | /    \ /
  a-------c
    \ C  /
     \  /
      \/

  (a,b,c) - current triangle
  points order: a -> b -> c
  neighbours order: A -> B -> C
*/

class MeshTriangle;
using TriangleHandle = std::weak_ptr<MeshTriangle>;

class MATH_CORE_API MeshTriangle : public Triangle
{
public:
    MeshTriangle(const Point3D& i_point1, const Point3D& i_point2, const Point3D& i_point3);

    ~MeshTriangle() override;

    void SetNeighbour(TriangleHandle i_neighbour);
    void SetNeighbour(TriangleHandle i_neighbour, short i_index);

    TriangleHandle GetNeighbour(short i_index) const;

    void RemoveNeighbour(short i_index);
    void RemoveNeighbour(const Triangle& i_neighbour);

    // Do NOT use this method dirctly, it can't update referenced mesh points. Should be called though mesh
    void SetPoint(short i_index, const Point3D& i_new_point) override;

private:
    std::array<TriangleHandle, 3> m_neighbours;
};
