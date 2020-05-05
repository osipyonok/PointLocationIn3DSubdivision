#include "Math.Core/MeshTriangle.h"

#include "Math.Core/Point3D.h"

#include <QtGlobal>

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

void MeshTriangle::SetNeighbour(TriangleHandle i_neighbour)
{
    if (_IsNeighbourAt(*this, *i_neighbour.lock(), 0))
        SetNeighbour(i_neighbour, 0);
    else if (_IsNeighbourAt(*this, *i_neighbour.lock(), 1))
        SetNeighbour(i_neighbour, 1);
    else if (_IsNeighbourAt(*this, *i_neighbour.lock(), 2))
        SetNeighbour(i_neighbour, 2);
    else
        Q_ASSERT_X(false, "MeshTriangle::SetNeighbour", "Given triangle is not a neighbour!");
}

void MeshTriangle::SetNeighbour(TriangleHandle i_neighbour, short i_index)
{
    Q_ASSERT(i_index >= 0 && i_index < 3);
    Q_ASSERT(_IsNeighbourAt(*this, *i_neighbour.lock(), i_index));

    m_neighbours[i_index].push_back(i_neighbour);
}

const std::list<TriangleHandle>& MeshTriangle::GetNeighbours(short i_index) const
{
    Q_ASSERT(i_index >= 0 && i_index < 3);

    return m_neighbours[i_index];
}

void MeshTriangle::RemoveAllNeighbours(short i_index)
{
    Q_ASSERT(i_index >= 0 && i_index < 3);

    m_neighbours[i_index].clear();
}

void MeshTriangle::RemoveNeighbour(const Triangle& i_neighbour)
{
    auto predicate = [&](const TriangleHandle& i_handle)
    {
        if (auto p_triangle = i_handle.lock())
        {
            return *p_triangle == i_neighbour;
        }
        else
        {
            Q_ASSERT(false);
            return false;
        }
    };

    auto it0 = std::find_if(m_neighbours[0].begin(), m_neighbours[0].end(), predicate);
    auto it1 = std::find_if(m_neighbours[1].begin(), m_neighbours[1].end(), predicate);
    auto it2 = std::find_if(m_neighbours[2].begin(), m_neighbours[2].end(), predicate);

    if (it0 != m_neighbours[0].end())
        m_neighbours[0].erase(it0);

    if (it1 != m_neighbours[1].end())
        m_neighbours[1].erase(it1);

    if (it2 != m_neighbours[2].end())
        m_neighbours[2].erase(it2);
}

void MeshTriangle::SetPoint(short i_index, const Point3D& i_new_point)
{
    const Point3D old_point = GetPoint(i_index);
    if (old_point == i_new_point)
        return;

    Triangle::SetPoint(i_index, i_new_point);
}
