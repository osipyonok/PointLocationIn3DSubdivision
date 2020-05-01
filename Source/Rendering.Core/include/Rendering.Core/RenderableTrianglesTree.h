#pragma once

#include <Rendering.Core/API.h>

#include <Rendering.Core/IRenderable.h>

#include <Math.DataStructures/TrianglesOctree.h>
#include <Math.DataStructures/TrianglesTree.h>

#include <memory>

class QColor;

class TransformMatrix;

namespace Rendering
{
    struct RENDERING_CORE_API ITreeDataSource
    {
        virtual ~ITreeDataSource() = default;

        virtual size_t GetMaxDepth() const = 0;
        virtual std::vector<BoundingBox> GetNodesBoxes(size_t i_layer) const = 0;
    };

    struct RENDERING_CORE_API TrianglesTreeDataSource : public ITreeDataSource
    {
        TrianglesTreeDataSource(const TrianglesTree& i_tree);

        size_t GetMaxDepth() const override;
        std::vector<BoundingBox> GetNodesBoxes(size_t i_layer) const override;

    private:
        const TrianglesTree* mp_tree = nullptr;
    };

    struct RENDERING_CORE_API TriangleOcTreeDataSource : public ITreeDataSource
    {
        TriangleOcTreeDataSource(const TrianglesOcTree& i_tree);

        size_t GetMaxDepth() const override;
        std::vector<BoundingBox> GetNodesBoxes(size_t i_layer) const override;

    private:
        const TrianglesOcTree* mp_tree = nullptr;
    };

    class RENDERING_CORE_API RenderableTrianglesTree : public IRenderable
    {
        Q_OBJECT
            
    public:
        enum class RenderingStyle
        {
            Opaque,
            Transparent,
        };

        RenderableTrianglesTree(std::unique_ptr<ITreeDataSource>&& ip_data_source);
        ~RenderableTrianglesTree() override;

        std::unique_ptr<Qt3DCore::QComponent> GetMaterial() const override;
        std::unique_ptr<Qt3DCore::QTransform> GetTransformation() const override;
        std::unique_ptr<Qt3DCore::QComponent> GetRenderer() const override;

        size_t GetLayersCount() const;
        size_t GetCurrentLayer() const;
        void SetCurrentLayer(size_t i_layer);

        void SetRenderingStyle(RenderingStyle i_style);
        RenderingStyle GetRenderingStyle() const;

        void SetColor(const QColor& i_color)  override;
        QColor GetColor() const override;
        void Transform(const TransformMatrix& i_transform) override;
        const TransformMatrix& GetTransform() const  override;

        std::vector<IRenderable*> GetNestedRenderables() const override;

    private:
        void _UpdateNestedRenderables();

        struct Impl;
        std::unique_ptr<Impl> mp_impl;
    };
}
