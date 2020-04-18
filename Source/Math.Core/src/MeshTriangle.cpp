#include "Math.Core/MeshTriangle.h"

#include "Math.Core/Point3D.h"

namespace
{
    bool _IsNeighbourAt(const Triangle& i_triangle, const Triangle& i_neighbour_candidate, short i_index)
    {
        auto current_edge_a = i_triangle.GetPoint(i_index);

        short index_of_a_in_neighbour = -1;
        if (i_neighbour_candidate.GetPoint(0) == current_edge_a)
            index_of_a_in_neighbour = 0;
        else if (i_neighbour_candidate.GetPoint(1) == current_edge_a)
            index_of_a_in_neighbour = 1;
        else if (i_neighbour_candidate.GetPoint(2) == current_edge_a)
            index_of_a_in_neighbour = 2;
        else
            return false;

        auto current_edge_b = i_triangle.GetPoint((i_index + 1) % 3);

        return current_edge_b == i_neighbour_candidate.GetPoint((index_of_a_in_neighbour + 1) % 3)
            || current_edge_b == i_neighbour_candidate.GetPoint((index_of_a_in_neighbour + 2) % 3);
    }
}

MeshTriangle::MeshTriangle(const Point3D& i_point1, const Point3D& i_point2, const Point3D& i_point3)
    : Triangle(i_point1, i_point2, i_point3)
{

}

MeshTriangle::~MeshTriangle() = default;

void MeshTriangle::SetNeighbour(const Triangle& i_neighbour)
{
    if (_IsNeighbourAt(*this, i_neighbour, 0))
        SetNeighbour(i_neighbour, 0);
    else if (_IsNeighbourAt(*this, i_neighbour, 1))
        SetNeighbour(i_neighbour, 1);
    else if (_IsNeighbourAt(*this, i_neighbour, 2))
        SetNeighbour(i_neighbour, 2);
    else
        Q_ASSERT_X(false, "MeshTriangle::SetNeighbour", "Given triangle is not a neighbour!");
}

void MeshTriangle::SetNeighbour(const Triangle& i_neighbour, short i_index)
{
    Q_ASSERT(i_index >= 0 && i_index < 3);
    Q_ASSERT(_IsNeighbourAt(*this, i_neighbour, i_index));

    m_neighbours[i_index] = std::make_unique<Triangle>(i_neighbour);
}

boost::optional<Triangle> MeshTriangle::GetNeighbour(short i_index) const
{
    Q_ASSERT(i_index >= 0 && i_index < 3);

    return m_neighbours[i_index].get() ? *m_neighbours[i_index] : boost::optional<Triangle>{};
}

void MeshTriangle::RemoveNeighbour(short i_index)
{
    Q_ASSERT(i_index >= 0 && i_index < 3);

    m_neighbours[i_index].reset();
}

void MeshTriangle::RemoveNeighbour(const Triangle& i_neighbour)
{
    if (m_neighbours[0]  && *m_neighbours[0] == i_neighbour)
        RemoveNeighbour(0);
    if (m_neighbours[1] && *m_neighbours[1] == i_neighbour)
        RemoveNeighbour(1);
    if (m_neighbours[2] && *m_neighbours[2] == i_neighbour)
        RemoveNeighbour(2);
}
