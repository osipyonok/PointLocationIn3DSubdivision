#include "Rendering.Core/RenderableTrianglesTree.h"

#include "Rendering.Core/RenderableBox.h"

#include <Math.Core/BoundingBox.h>
#include <Math.Core/Mesh.h>
#include <Math.Core/TransformMatrix.h>
#include <Math.Core/Vector3D.h>

#include <Klein/Render/UnlitSolidWireframeMaterial.h>
#include <Klein/Render/WBOITMaterial.h>

#include <QColor>

#include <Qt3DCore/QTransform>

#include <functional>
#include <random>


namespace
{
    size_t _GetMaxLayersCount(const TrianglesTree& i_tree)
    {
        if (!i_tree.WasBuild())
            return 0;

        std::function<size_t(const TrianglesTree::NodeType&)> depth_counter = [&](const TrianglesTree::NodeType& i_node)
        {
            size_t left_count = 0;
            size_t right_count = 0;

            if (i_node.HasLeftChild())
            {
                left_count = depth_counter(const_cast<TrianglesTree::NodeType&>(i_node).GetLeftChild());
            }

            if (i_node.HasRightChild())
            {
                right_count = depth_counter(const_cast<TrianglesTree::NodeType&>(i_node).GetRightChild());
            }

            return std::max(left_count, right_count) + 1;
        };

        return depth_counter(const_cast<TrianglesTree&>(i_tree).GetRoot());
    }

    std::vector<BoundingBox> _GetBBoxes(const TrianglesTree& i_tree, size_t i_depth_limit)
    {
        std::vector<BoundingBox> boxes;

        std::function<void(const TrianglesTree::NodeType&, size_t)> bbox_collector = [&](const TrianglesTree::NodeType& i_node, size_t i_depth)
        {
            if (i_depth == i_depth_limit || (!i_node.HasLeftChild() && !i_node.HasRightChild()))
            {
                boxes.emplace_back(i_node.GetInfo().m_bbox);
                return;
            }

            if (i_depth > i_depth_limit)
            {
                Q_ASSERT(false);
                return;
            }

            if (i_node.HasLeftChild())
            {
                bbox_collector(const_cast<TrianglesTree::NodeType&>(i_node).GetLeftChild(), i_depth + 1);
            }

            if (i_node.HasRightChild())
            {
                bbox_collector(const_cast<TrianglesTree::NodeType&>(i_node).GetRightChild(), i_depth + 1);
            }
        };

        bbox_collector(const_cast<TrianglesTree&>(i_tree).GetRoot(), 0);
        return std::move(boxes);
    }

    std::vector<QColor> _GenerateNColor(size_t i_n)
    {
        static constexpr int seed = 555;
        std::mt19937 random_generator(seed);

        std::vector<QColor> colors;
        colors.reserve(i_n);
        for (size_t i = 0; i < i_n; ++i)
            colors.emplace_back(QColor(random_generator() % 256, random_generator() % 256, random_generator() % 256));

        return std::move(colors);
    }
}


namespace Rendering
{

    struct RenderableTrianglesTree::Impl 
    {
        const TrianglesTree* mp_tree = nullptr;
        size_t m_layers_count = 0;
        size_t m_current_layer = 0;
        RenderingStyle m_style = RenderingStyle::Transparent;
        std::vector<std::unique_ptr<RenderableBox>> m_renderables;
        TransformMatrix m_transform;
    };

    RenderableTrianglesTree::RenderableTrianglesTree(const TrianglesTree& i_tree)
        : mp_impl(std::make_unique<Impl>())
    {
        Q_ASSERT(i_tree.WasBuild());
        mp_impl->mp_tree = &i_tree;
        mp_impl->m_layers_count = _GetMaxLayersCount(i_tree);
        _UpdateNestedRenderables();
    }

    RenderableTrianglesTree::~RenderableTrianglesTree()
    {
        _OnDestructed();
    }

    std::unique_ptr<Qt3DCore::QComponent> RenderableTrianglesTree::GetMaterial() const
    {
        return nullptr;
    }

    std::unique_ptr<Qt3DCore::QTransform> RenderableTrianglesTree::GetTransformation() const
    {
        return nullptr;
    }

    std::unique_ptr<Qt3DCore::QComponent> RenderableTrianglesTree::GetRenderer() const
    {
        return nullptr;
    }

    size_t RenderableTrianglesTree::GetLayersCount() const
    {
        return mp_impl->m_layers_count;
    }

    size_t RenderableTrianglesTree::GetCurrentLayer() const
    {
        return mp_impl->m_current_layer;
    }

    void RenderableTrianglesTree::SetCurrentLayer(size_t i_layer)
    {
        if (i_layer == mp_impl->m_current_layer)
            return;

        mp_impl->m_current_layer = i_layer;
        _UpdateNestedRenderables();
    }

    void RenderableTrianglesTree::SetRenderingStyle(RenderingStyle i_style)
    {
        if (i_style == mp_impl->m_style)
            return;

        mp_impl->m_style = i_style;
        for (auto& p_renderable : mp_impl->m_renderables)
        {
            if (mp_impl->m_style == RenderingStyle::Opaque)
                p_renderable->SetRenderingStyle(RenderableBox::RenderingStyle::Opaque);
            else
                p_renderable->SetRenderingStyle(RenderableBox::RenderingStyle::Transparent);
        }

        emit RenderableMaterialChanged();
    }

    RenderableTrianglesTree::RenderingStyle RenderableTrianglesTree::GetRenderingStyle() const
    {
        return mp_impl->m_style;
    }

    void RenderableTrianglesTree::SetColor(const QColor& i_color)
    {
        Q_UNUSED(i_color);
    }

    QColor RenderableTrianglesTree::GetColor() const
    {
        return QColor("black");
    }

    void RenderableTrianglesTree::Transform(const TransformMatrix& i_transform)
    {
        if (i_transform == TransformMatrix{})
            return;

        for (auto& p_renderable : mp_impl->m_renderables)
            p_renderable->Transform(i_transform);

        emit RenderableTransformationChanged();
    }

    const TransformMatrix& RenderableTrianglesTree::GetTransform() const
    {
        return mp_impl->m_transform;
    }

    std::vector<IRenderable*> RenderableTrianglesTree::GetNestedRenderables() const
    {
        std::vector<IRenderable*> renderables;
        for (const auto& p_renderable : mp_impl->m_renderables)
            renderables.push_back(p_renderable.get());

        return std::move(renderables);
    }

    void RenderableTrianglesTree::_UpdateNestedRenderables()
    {
        emit NestedRenderablesAboutToBeReset();
        mp_impl->m_renderables.clear();

        std::vector<BoundingBox> boxes = _GetBBoxes(*mp_impl->mp_tree, mp_impl->m_current_layer);
        auto colors = _GenerateNColor(boxes.size());

        for (const auto& bbox : boxes)
        {
            auto p_renderable = std::make_unique<RenderableBox>(bbox);
            p_renderable->Transform(mp_impl->m_transform);

            if (mp_impl->m_style == RenderingStyle::Opaque)
                p_renderable->SetRenderingStyle(RenderableBox::RenderingStyle::Opaque);
            else
                p_renderable->SetRenderingStyle(RenderableBox::RenderingStyle::Transparent);

            p_renderable->SetColor(colors.back());
            colors.pop_back();

            mp_impl->m_renderables.emplace_back(std::move(p_renderable));
        }

        emit NestedRenderablesReset();
    }

}

