#pragma once

#include <Math.Core/BoundingBox.h>
#include <Math.Core/Point3D.h>

#include <array>
#include <cassert>
#include <functional>
#include <memory>

template<typename Info>
class OcTreeNode final
{
public:
    using NodeType = OcTreeNode<Info>;
    using InfoType = Info;

    OcTreeNode(const BoundingBox& i_bbox)
        : m_bbox(i_bbox)
    {
    }

    ~OcTreeNode() = default;

    const NodeType* GetChild(size_t i_index) const;
    NodeType* GetOrCreateChild(size_t i_index);

    InfoType& GetInfo();
    const InfoType& GetInfo() const;

    BoundingBox GetPotentialChildBBox(size_t i_index) const;
    const BoundingBox& GetBoundingBox() const { return m_bbox; }

private:
    BoundingBox m_bbox;
    std::unique_ptr<InfoType> mp_info = std::make_unique<InfoType>();
    std::array<std::unique_ptr<NodeType>, 8> m_children;
};

// build functor: void(Node, Args...). params: root node and extra args needed for construction
// query functor: void(Node, Result&, Args...). params: result output parameter, extra args
// bbox  functor: void(BBox&, Args...). updates bbox with the given args
template<typename Info, typename BuildFunctor, typename QueryFunctor, typename BBoxUpdater>
class GenericOcTree final
{
public:
    using NodeType = OcTreeNode<Info>;
    using BBoxUpdaterType = BBoxUpdater;

    template<typename... Args>
    void Build(Args&&... i_args);

    template<typename Result, typename... Args>
    void Query(Result& o_result, Args&&... i_args);

    NodeType& GetRoot() { return *mp_root; }

    bool WasBuild() const { return mp_root != nullptr; }

private:
    std::unique_ptr<NodeType> mp_root;
};

template<typename Info>
inline const typename OcTreeNode<Info>::NodeType* OcTreeNode<Info>::GetChild(size_t i_index) const
{
    return m_children[i_index].get();
}

template<typename Info>
inline typename OcTreeNode<Info>::NodeType* OcTreeNode<Info>::GetOrCreateChild(size_t i_index)
{
    if (!m_children[i_index])
    {
        auto child_bbox = GetPotentialChildBBox(i_index);
        m_children[i_index] = std::make_unique<typename OcTreeNode<Info>::NodeType>(child_bbox);
    }

    return m_children[i_index].get();
}

template<typename Info>
inline typename OcTreeNode<Info>::InfoType& OcTreeNode<Info>::GetInfo()
{
    return *mp_info;
}

template<typename Info>
inline const typename OcTreeNode<Info>::InfoType& OcTreeNode<Info>::GetInfo() const
{
    return *mp_info;
}

template<typename Info>
inline BoundingBox OcTreeNode<Info>::GetPotentialChildBBox(size_t i_index) const
{
    auto center = (m_bbox.GetMin() + m_bbox.GetMax()) / 2;

    auto min_corner = center;
    auto max_corner = center;

    // x coord
    if (i_index == 0 || i_index == 2 || i_index == 4 || i_index == 6)
    {
        min_corner.SetX(m_bbox.GetMin().GetX());
    }
    else
    {
        max_corner.SetX(m_bbox.GetMax().GetX());
    }

    // y coord
    if (i_index == 0 || i_index == 1 || i_index == 4 || i_index == 5)
    {
        min_corner.SetY(m_bbox.GetMin().GetY());
    }
    else
    {
        max_corner.SetY(m_bbox.GetMax().GetY());
    }

    // z coord
    if (i_index == 0 || i_index == 1 || i_index == 2 || i_index == 3)
    {
        min_corner.SetZ(m_bbox.GetMin().GetZ());
    }
    else
    {
        max_corner.SetZ(m_bbox.GetMax().GetZ());
    }

    BoundingBox child_bbox;
    child_bbox.AddPoint(min_corner);
    child_bbox.AddPoint(max_corner);

    return child_bbox;
}

template<typename Info, typename BuildFunctor, typename QueryFunctor, typename BBoxUpdater>
template<typename... Args>
inline void GenericOcTree<Info, BuildFunctor, QueryFunctor, BBoxUpdater>::Build(Args&&... i_args)
{
    BoundingBox bbox;
    BBoxUpdater updater;
    updater(bbox, i_args...);

    mp_root = std::make_unique<NodeType>(bbox);

    std::invoke(BuildFunctor{}, *mp_root, std::forward<Args>(i_args)...);
}

template<typename Info, typename BuildFunctor, typename QueryFunctor, typename BBoxUpdater>
template<typename Result, typename ...Args>
inline void GenericOcTree<Info, BuildFunctor, QueryFunctor, BBoxUpdater>::Query(Result& o_result, Args&& ...i_args)
{
    if (!WasBuild())
    {
        assert(false);
        return;
    }

    std::invoke(QueryFunctor{}, *mp_root, o_result, std::forward<Args>(i_args)...);
}
