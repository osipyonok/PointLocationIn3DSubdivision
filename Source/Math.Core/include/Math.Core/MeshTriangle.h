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

class MATH_CORE_API MeshTriangle : public Triangle
{
    Q_OBJECT

public:
    MeshTriangle(const Point3D& i_point1, const Point3D& i_point2, const Point3D& i_point3);

    ~MeshTriangle() override;

    void SetNeighbour(const Triangle& i_neighbour);
    void SetNeighbour(const Triangle& i_neighbour, short i_index);

    boost::optional<Triangle> GetNeighbour(short i_index) const;

    void RemoveNeighbour(short i_index);
    void RemoveNeighbour(const Triangle& i_neighbour);

private:
    std::array<std::unique_ptr<Triangle>, 3> m_neighbours;
};
