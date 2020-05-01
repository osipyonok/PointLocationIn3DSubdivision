#pragma once

#include <functional>
#include <memory>

template<typename Info>
class KDTreeNode final
{
public:
    using NodeType = KDTreeNode<Info>;

    KDTreeNode()  = default;
    ~KDTreeNode() = default;

    NodeType& GetLeftChild();
    NodeType& GetRightChild();

    bool HasLeftChild() const;
    bool HasRightChild() const;

    Info& GetInfo();


private:
    std::unique_ptr<NodeType> mp_left_child;
    std::unique_ptr<NodeType> mp_right_child;
    std::unique_ptr<Info> mp_info = std::make_unique<Info>();
};


// build functor: void(Node, Args...). params: root node and extra args needed for construction
// query functor: void(Node, Result&, Args...). params: result output parameter, extra args
template<typename Info, typename BuildFunctor, typename QueryFunctor>
class GenericKDTree final
{
public:
    using NodeType = KDTreeNode<Info>;

    template<typename... Args>
    void Build(Args&&... i_args);

    template<typename Result, typename... Args>
    void Query(Result& o_result, Args&&... i_args);

    NodeType& GetRoot() { return m_root; }

    bool WasBuild() const { return m_was_build; }

private:
    NodeType m_root;
    bool m_was_build = false;
};



template<typename Info>
typename KDTreeNode<Info>::NodeType& KDTreeNode<Info>::GetLeftChild()
{
    if (!HasLeftChild())
        mp_left_child = std::make_unique<NodeType>();

    return *mp_left_child;
}

template<typename Info>
inline typename KDTreeNode<Info>::NodeType& KDTreeNode<Info>::GetRightChild()
{
    if (!HasRightChild())
        mp_right_child = std::make_unique<NodeType>();

    return *mp_right_child;
}

template<typename Info>
inline bool KDTreeNode<Info>::HasLeftChild() const
{
    return mp_left_child != nullptr;
}

template<typename Info>
inline bool KDTreeNode<Info>::HasRightChild() const
{
    return mp_right_child != nullptr;
}

template<typename Info>
inline Info& KDTreeNode<Info>::GetInfo()
{
    return *mp_info;
}

template<typename Info, typename BuildFunctor, typename QueryFunctor>
template<typename... Args>
inline void GenericKDTree<Info, BuildFunctor, QueryFunctor>::Build(Args&&... i_args)
{
    std::invoke(BuildFunctor{}, m_root, std::forward<Args>(i_args)...);
    m_was_build = true;
}

template<typename Info, typename BuildFunctor, typename QueryFunctor>
template<typename Result, typename... Args>
inline void GenericKDTree<Info, BuildFunctor, QueryFunctor>::Query(Result& o_result, Args&&... i_args)
{
    std::invoke(QueryFunctor{}, m_root, o_result, std::forward<Args>(i_args)...);
}
