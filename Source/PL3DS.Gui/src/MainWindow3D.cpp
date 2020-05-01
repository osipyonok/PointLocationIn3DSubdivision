#include "PL3DS.Gui/MainWindow3D.h"

#include <Rendering.Core/IRenderable.h>

#include <Rendering.Main/RenderablesController.h>

#include <Klein/Input/TrackballCameraController.h>
#include <Klein/Render/UnlitMaterial.h>
#include <Klein/Render/WBOITMaterial.h>
#include <imgui.h>

#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QCameraSelector>
#include <Qt3DRender/QClearBuffers>
#include <Qt3DRender/QColorMask>
#include <Qt3DRender/QFrameGraphNode>
#include <Qt3DRender/QMesh>
#include <Qt3DRender/QNoDraw>
#include <Qt3DRender/QRenderStateSet>
#include <Qt3DRender/QRenderSettings>
#include <Qt3DRender/QRenderSurfaceSelector>
#include <Qt3DRender/QViewport>

#include <QColor>
#include <QPointer>
#include <QVector3D>

#include <Qt3DCore/QAspectEngine>
#include <Qt3DInput/QInputSettings>

#include <QCoreApplication>
#include <QString>

#include <unordered_map>

namespace
{
    const auto _property_mesh = "PROPERTY_MAIN_WINDOW_MESH";

    struct RenderableData
    {
        QPointer<Qt3DCore::QComponent> mp_material;
        QPointer<Qt3DCore::QComponent> mp_transformation;
        QPointer<Qt3DCore::QComponent> mp_renderer;

        QPointer<Qt3DCore::QEntity> mp_mesh;
    };
}

namespace UI
{
    struct MainWindow3D::Impl
    {
        QPointer<Qt3DRender::QCamera>              mp_camera;
        QPointer<Klein::TrackballCameraController> mp_camera_controller;
        QPointer<Klein::WBOITCompositor>           mp_compositor;

        mutable std::unordered_map<const Rendering::IRenderable*, RenderableData> m_renderables_cache;
    };

    MainWindow3D::MainWindow3D(QWindow* ip_parent /*= nullptr*/)
        : Klein::AbstractQt3DWindow(ip_parent)
        , mp_impl(std::make_unique<Impl>())
    {
        using Rendering::RenderablesController;
        auto p_controller = &RenderablesController::GetInstance();
        bool is_connected = connect(p_controller, &RenderablesController::RenderableAdded, this, &MainWindow3D::_OnRenderableAdded);
        Q_ASSERT(is_connected);

        is_connected = connect(p_controller, &RenderablesController::RenderableRemoved, this, &MainWindow3D::_OnRenderableRemoved);
        Q_ASSERT(is_connected);

        is_connected = connect(p_controller, &RenderablesController::RenderableMaterialChanged, this, &MainWindow3D::_OnRenderableMaterialChanged);
        Q_ASSERT(is_connected);

        is_connected = connect(p_controller, &RenderablesController::RenderableRendererChanged, this, &MainWindow3D::_OnRenderableRendererChanged);
        Q_ASSERT(is_connected);

        is_connected = connect(p_controller, &RenderablesController::RenderableTransformationChanged, this, &MainWindow3D::_OnRenderableTransformationChanged);
        Q_ASSERT(is_connected);

        is_connected = connect(p_controller, &RenderablesController::NestedRenderablesAboutToBeReset, this, &MainWindow3D::_OnNestedRenderableAboutToBeReset);
        Q_ASSERT(is_connected);

        is_connected = connect(p_controller, &RenderablesController::NestedRenderablesReset, this, &MainWindow3D::_OnNestedRenderableReset);
        Q_ASSERT(is_connected);

        Q_UNUSED(is_connected);
    }

    MainWindow3D::~MainWindow3D() = default;

    void MainWindow3D::ViewAll()
    {
        if (mp_impl->mp_camera)
            mp_impl->mp_camera->viewAll();
    }

    void MainWindow3D::ViewRenderable(const Rendering::IRenderable* ip_renderable)
    {
        if (!ip_renderable)
            return;

        auto it = mp_impl->m_renderables_cache.find(ip_renderable);
        if (it == mp_impl->m_renderables_cache.end())
            return;

        if (mp_impl->mp_camera)
            mp_impl->mp_camera->viewEntity(it->second.mp_mesh.data());
    }

    void MainWindow3D::resizeEvent(QResizeEvent*)
    {
        if (mp_impl->mp_camera)
            mp_impl->mp_camera->setAspectRatio(static_cast<float>(width()) / height());

        if (mp_impl->mp_camera_controller)
            mp_impl->mp_camera_controller->setWindowSize(size());

        if (mp_impl->mp_compositor)
            mp_impl->mp_compositor->setSize(size());
    }

    Qt3DCore::QEntity* MainWindow3D::createSceneGraph()
    {
        auto rootEntity = new Qt3DCore::QEntity;

        _CreateMeshes(rootEntity);

        // WBOIT material also needs an offscreen render target
        mp_impl->mp_compositor = new Klein::WBOITCompositor(rootEntity);
        mp_impl->mp_compositor->setSize(size());

        mp_impl->mp_camera = new Qt3DRender::QCamera(rootEntity);
        mp_impl->mp_camera->setPosition(QVector3D(1.f, 1.f, 1.f));
        mp_impl->mp_camera->setViewCenter(QVector3D(0, 0, 0));
        auto aspect = (this->width() + 0.0f) / this->height();
        mp_impl->mp_camera->lens()->setPerspectiveProjection(60.0f, aspect, 0.1f, 100.0f);

        mp_impl->mp_camera_controller = new Klein::TrackballCameraController(rootEntity);
        mp_impl->mp_camera_controller->setCamera(mp_impl->mp_camera);
        mp_impl->mp_camera_controller->setWindowSize(size());
        
        return rootEntity;
    }

    Qt3DRender::QRenderSettings* MainWindow3D::createRenderSettings(Qt3DCore::QEntity* root)
    {
        auto rootNode = new Qt3DRender::QFrameGraphNode(root);

        auto surfaceSelector = new Qt3DRender::QRenderSurfaceSelector(rootNode);
        surfaceSelector->setSurface(this);

        auto viewport = new Qt3DRender::QViewport(surfaceSelector);
        viewport->setNormalizedRect(QRect(0, 0, 1, 1));

        // Clear default framebuffer
        {
            auto clearBuffers = new Qt3DRender::QClearBuffers(viewport);
            clearBuffers->setBuffers(Qt3DRender::QClearBuffers::ColorDepthBuffer);
            clearBuffers->setClearColor(QColor(255, 255, 255));

            new Qt3DRender::QNoDraw(clearBuffers);
        }

        // Render opaque entities first
        {
            auto cameraSelector = new Qt3DRender::QCameraSelector(viewport);
            cameraSelector->setCamera(mp_impl->mp_camera);

            Klein::BaseUnlitMaterial::attachRenderPassTo(cameraSelector);
        }

        // Render transparent entities
        {
            auto cameraSelector = new Qt3DRender::QCameraSelector(viewport);
            cameraSelector->setCamera(mp_impl->mp_camera);

            // Render transparency to a composition target
            {
                auto target = mp_impl->mp_compositor->attachRenderTargetTo(cameraSelector);

                // FIXME:
                // Qt3D doesn't support blitting depth buffer
                // Here we render opaque objects again to get depth buffer
                // Could render to a fbo instead and copy to the default one
                auto colorMask = new Qt3DRender::QColorMask;
                colorMask->setRedMasked(false);
                colorMask->setGreenMasked(false);
                colorMask->setBlueMasked(false);
                colorMask->setAlphaMasked(false);
                auto renderStateSet = new Qt3DRender::QRenderStateSet(target);
                renderStateSet->addRenderState(colorMask);
                Klein::BaseUnlitMaterial::attachRenderPassTo(renderStateSet);

                Klein::WBOITMaterial::attachTranparentPassTo(target);
            }

            // Composite transparency to the on-screen target
            {
                mp_impl->mp_compositor->attachCompositionPassTo(cameraSelector);
            }
        }

        auto settings = new Qt3DRender::QRenderSettings(root);
        settings->setActiveFrameGraph(rootNode);
        return settings;
    }

    void MainWindow3D::_CreateMeshFor(const Rendering::IRenderable* ip_renderable, Qt3DCore::QEntity* ip_parent) const
    {
        for (const auto p_nested_renderable : ip_renderable->GetNestedRenderables())
            _CreateMeshFor(p_nested_renderable, ip_parent);

        auto p_renderer = ip_renderable->GetRenderer();
        auto p_material = ip_renderable->GetMaterial();
        auto p_transform = ip_renderable->GetTransformation();

        if (!p_renderer || !p_material || !p_transform)
            return;

        Q_ASSERT(p_renderer);
        Q_ASSERT(p_material);
        Q_ASSERT(p_transform);

        auto p_mesh = new Qt3DCore::QEntity(ip_parent);
        p_mesh->setProperty(_property_mesh, true);

        RenderableData cache;
        cache.mp_mesh = p_mesh;
        cache.mp_material = p_material.get();
        cache.mp_renderer = p_renderer.get();
        cache.mp_transformation = p_transform.get();
        mp_impl->m_renderables_cache[ip_renderable] = cache;

        p_renderer->setParent(qobject_cast<Qt3DCore::QNode*>(ip_parent));
        p_material->setParent(qobject_cast<Qt3DCore::QNode*>(ip_parent));
        p_transform->setParent(qobject_cast<Qt3DCore::QNode*>(ip_parent));

        p_mesh->addComponent(p_material.release());
        p_mesh->addComponent(p_renderer.release());
        p_mesh->addComponent(p_transform.release());
    }

    void MainWindow3D::_CreateMeshes(Qt3DCore::QEntity* ip_parent) const
    {
        const auto renderables = Rendering::RenderablesController::GetInstance().GetRenderables();
        for (auto p_renderable : renderables)
            _CreateMeshFor(p_renderable, ip_parent);
    }

    Qt3DCore::QEntity* MainWindow3D::_GetRootEntity() const
    {
        return m_aspectEngine->rootEntity() ? m_aspectEngine->rootEntity().get() : nullptr;
    }

    void MainWindow3D::_OnRenderableAdded(const Rendering::IRenderable* ip_renderable)
    {
        if (auto p_root = _GetRootEntity())
            _CreateMeshFor(ip_renderable, p_root);
    }

    void MainWindow3D::_OnRenderableRemoved(const Rendering::IRenderable* ip_renderable)
    {
        for (auto p_nested_renderable : ip_renderable->GetNestedRenderables())
            _OnRenderableRemoved(p_nested_renderable);

        if (mp_impl->m_renderables_cache.find(ip_renderable) == mp_impl->m_renderables_cache.end())
            return;

        auto& cached_data = mp_impl->m_renderables_cache[ip_renderable];

        cached_data.mp_mesh->setEnabled(false);

        if (auto p_root = m_aspectEngine->rootEntity())
        {
            cached_data.mp_mesh->deleteLater();
            cached_data.mp_transformation->deleteLater();
            cached_data.mp_material->deleteLater();
            cached_data.mp_renderer->deleteLater();
        }

        mp_impl->m_renderables_cache.erase(ip_renderable);
    }

    void MainWindow3D::_OnRenderableMaterialChanged(const Rendering::IRenderable* ip_renderable)
    {
        for (auto p_nested_renderable : ip_renderable->GetNestedRenderables())
            _OnRenderableMaterialChanged(p_nested_renderable);

        if (mp_impl->m_renderables_cache.find(ip_renderable) == mp_impl->m_renderables_cache.end())
            return;

        auto& cached_data = mp_impl->m_renderables_cache[ip_renderable];

        if (cached_data.mp_material && cached_data.mp_mesh)
            cached_data.mp_material->removedFromEntity(cached_data.mp_mesh.data());

        if (auto p_mesh = cached_data.mp_mesh)
        {
            auto p_new_material = ip_renderable->GetMaterial();
            p_new_material->setParent(qobject_cast<Qt3DCore::QNode*>(p_mesh));
            cached_data.mp_material = p_new_material.get();
            p_mesh->addComponent(p_new_material.release());
        }
    }

    void MainWindow3D::_OnRenderableTransformationChanged(const Rendering::IRenderable* ip_renderable)
    {
        for (auto p_nested_renderable : ip_renderable->GetNestedRenderables())
            _OnRenderableTransformationChanged(p_nested_renderable);

        if (mp_impl->m_renderables_cache.find(ip_renderable) == mp_impl->m_renderables_cache.end())
            return;

        auto& cached_data = mp_impl->m_renderables_cache[ip_renderable];

        if (cached_data.mp_transformation && cached_data.mp_mesh)
            cached_data.mp_transformation->removedFromEntity(cached_data.mp_mesh.data());

        if (auto p_mesh = cached_data.mp_mesh)
        {
            auto p_new_transform = ip_renderable->GetTransformation();
            p_new_transform->setParent(qobject_cast<Qt3DCore::QNode*>(p_mesh));
            cached_data.mp_transformation = p_new_transform.get();
            p_mesh->addComponent(p_new_transform.release());
        }
    }

    void MainWindow3D::_OnRenderableRendererChanged(const Rendering::IRenderable* ip_renderable)
    {
        for (auto p_nested_renderable : ip_renderable->GetNestedRenderables())
            _OnRenderableRendererChanged(p_nested_renderable);

        if (mp_impl->m_renderables_cache.find(ip_renderable) == mp_impl->m_renderables_cache.end())
            return;

        auto& cached_data = mp_impl->m_renderables_cache[ip_renderable];

        if (cached_data.mp_renderer && cached_data.mp_mesh)
            cached_data.mp_renderer->removedFromEntity(cached_data.mp_mesh.data());

        if (auto p_mesh = cached_data.mp_mesh)
        {
            auto p_new_renderer = ip_renderable->GetRenderer();
            p_new_renderer->setParent(qobject_cast<Qt3DCore::QNode*>(p_mesh));
            cached_data.mp_renderer = p_new_renderer.get();
            p_mesh->addComponent(p_new_renderer.release());
        }
    }

    void MainWindow3D::_OnNestedRenderableAboutToBeReset(const Rendering::IRenderable* ip_renderable)
    {
        for (auto p_old_renderable : ip_renderable->GetNestedRenderables())
            _OnRenderableRemoved(p_old_renderable);
    }

    void MainWindow3D::_OnNestedRenderableReset(const Rendering::IRenderable* ip_renderable)
    {
        for (auto p_new_renderable : ip_renderable->GetNestedRenderables())
            _OnRenderableAdded(p_new_renderable);
    }

}
