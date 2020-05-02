#include "PL3DS.Gui/Actions/Actions.h"

#include "PL3DS.Gui/RenderablesModel.h"

#include "PL3DS.Gui/Dialogs/AddPointDialog.h"
#include "PL3DS.Gui/Dialogs/LocalizeDialog.h"
#include "PL3DS.Gui/Dialogs/SubdivideDialog.h"
#include "PL3DS.Gui/Dialogs/TranslateDialog.h"

#include "PL3DS.Gui/Utilities/GeneralUtilities.h"
#include "PL3DS.Gui/Utilities/ProgressDialog.h"

#include <Math.Core/Point3D.h>
#include <Math.Core/Mesh.h>
#include <Math.Core/TransformMatrix.h>

#include <Math.IO/MeshIO.h>

#include <Math.Algos/Sqrt3Subdivision.h>
#include <Math.Algos/Voxelizer.h>

#include <Math.DataStructures/VoxelGrid.h>

#include <Rendering.Core/IRenderable.h>
#include <Rendering.Main/RenderableFactory.h>

#include <QAction>
#include <QFileDialog>
#include <QStandardPaths>

#include <list>
#include <memory>


namespace
{
    // such a things shouldn't be done in ideal world :(
    static Rendering::IRenderable* g_current_renderable = nullptr;
    static std::map<Rendering::IRenderable*, Mesh*> g_renderable_to_mesh_map;

    // standalone renderables
    static std::vector<std::unique_ptr<Rendering::RenderablePoint>> g_renderable_points;
    static std::vector<std::unique_ptr<Rendering::RenderableBox>> g_renderable_bboxes;
    static std::vector<std::unique_ptr<Rendering::RenderableVoxelGrid>> g_renderable_grids;

    class MeshDataBase
    {
    public:
        void AddMesh(std::unique_ptr<Mesh>&& ip_mesh);
        void RemoveMesh(Mesh* ip_mesh);
    private:
        std::list<std::unique_ptr<Mesh>> m_meshes;

    } g_mesh_data_base;

    void MeshDataBase::AddMesh(std::unique_ptr<Mesh>&& ip_mesh)
    {
        Q_ASSERT(ip_mesh);
        m_meshes.emplace_back(std::move(ip_mesh));
    }

    void MeshDataBase::RemoveMesh(Mesh* ip_mesh)
    {
        auto it = std::find_if(m_meshes.begin(), m_meshes.end(), [ip_mesh](const std::unique_ptr<Mesh>& ip_unique_mesh)
        {
            return ip_unique_mesh.get() == ip_mesh;
        });

        if (it != m_meshes.end())
            m_meshes.erase(it);
    }

    void _DoLoadInThreadImpl(const QStringList& i_paths)
    {
        std::vector<std::pair<Rendering::IRenderable*, QString>> new_renderables;
        auto loader = [&]
        {
            for (const auto& path : i_paths)
            {
                auto p_mesh = std::make_unique<Mesh>();
                auto result = ReadMesh(path, *p_mesh);
                if (result)
                {
                    auto p_renderable = Rendering::CreateRenderableFor(*p_mesh);
                    p_renderable->setParent(p_mesh.get());
                    p_renderable->SetColor(Utilities::GenerateRandomColor());
                    g_renderable_to_mesh_map[p_renderable.get()] = p_mesh.get();
                    new_renderables.emplace_back(p_renderable.get(), p_mesh->GetName());
                    p_renderable.release();
                    g_mesh_data_base.AddMesh(std::move(p_mesh));
                }
            }
        };

        UI::RunInThread(loader, "Load Mesh");

        for (const auto& renderable : new_renderables)
            RenderablesModel::GetInstance().AddRenderable(renderable.first, renderable.second);
    }

    void _DoLoadMesh()
    {
        auto file_paths = QFileDialog::getOpenFileNames(nullptr, "Load Mesh", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), 
                                                        "Model files (*.stl *.obj)");
        if (file_paths.isEmpty())
            return;

        _DoLoadInThreadImpl(file_paths);
    }

    void _DoRemoveSelected()
    {
        auto p_current_renderable = Actions::Configuration::GetCurrentRenderable();
        if (!p_current_renderable)
            return;

        RenderablesModel::GetInstance().RemoveRenderable(p_current_renderable);
        if (g_renderable_to_mesh_map.find(p_current_renderable) != g_renderable_to_mesh_map.end())
        {
            auto p_mesh = g_renderable_to_mesh_map[p_current_renderable];
            g_renderable_to_mesh_map.erase(p_current_renderable);
            g_mesh_data_base.RemoveMesh(p_mesh);
        }

        auto it_point = std::find_if(g_renderable_points.begin(), g_renderable_points.end(), [p_current_renderable](auto& ip_renderable)
        {
            return ip_renderable.get() == p_current_renderable;
        });
        if (it_point != g_renderable_points.end())
        {
            g_renderable_points.erase(it_point);
        }

        auto it_bbox = std::find_if(g_renderable_bboxes.begin(), g_renderable_bboxes.end(), [p_current_renderable](auto& ip_renderable)
        {
            return ip_renderable.get() == p_current_renderable;
        });
        if (it_bbox != g_renderable_bboxes.end())
        {
            g_renderable_bboxes.erase(it_bbox);
        }

        auto it_grid = std::find_if(g_renderable_grids.begin(), g_renderable_grids.end(), [p_current_renderable](auto& ip_renderable)
        {
            return ip_renderable.get() == p_current_renderable;
        });
        if (it_grid != g_renderable_grids.end())
        {
            g_renderable_grids.erase(it_grid);
        }
    }

    void _DoVoxelize()
    {
        auto p_current_renderable = Actions::Configuration::GetCurrentRenderable();
        if (!p_current_renderable)
            return;

        if (g_renderable_to_mesh_map.find(p_current_renderable) == g_renderable_to_mesh_map.end())
            return;

        auto p_mesh = g_renderable_to_mesh_map[p_current_renderable];

        Voxelizer::Params params;
        params.m_resolution_x = params.m_resolution_y = params.m_resolution_z = 0.2;

        Rendering::IRenderable* p_new_renderable = nullptr;
        auto voxelizer = [&]
        {
            Voxelizer voxelizer;
            voxelizer.SetParams(params);
            auto p_voxelized_mesh = std::make_unique<Mesh>();
            auto p_voxel_grid = voxelizer.Voxelize(*p_mesh, *p_voxelized_mesh);

            if (p_voxel_grid)
            {
                auto p_renderable_grid = Rendering::CreateRenderableFor(*p_voxel_grid);
                p_renderable_grid->Transform(p_current_renderable->GetTransform());
                p_renderable_grid->SetRenderingStyle(Rendering::RenderableVoxelGrid::RenderingStyle::Transparent);
                p_new_renderable = p_renderable_grid.get();
                g_renderable_grids.emplace_back(std::move(p_renderable_grid));
            }
        };

        RenderablesModel::GetInstance().AddRenderable(p_new_renderable, p_mesh->GetName() + QStringLiteral("_Voxelization"));

        UI::RunInThread(voxelizer, "Voxelization");
    }

    void _DoAddPoint()
    {
        UI::AddPointParameters params;
        UI::AddPointDialog dialog(params);
        dialog.setWindowFlag(Qt::WindowType::WindowContextHelpButtonHint, false);
        if (dialog.exec() == QDialog::Accepted)
        {
            Point3D point(params.m_x, params.m_y, params.m_z);
            auto p_renderable = Rendering::CreateRenderableFor(point);
            auto name = QStringLiteral("Point at (%1,%2,%3)").arg(QString::number(point.GetX(), 'f'))
                                                             .arg(QString::number(point.GetY(), 'f'))
                                                             .arg(QString::number(point.GetZ(), 'f'));
            RenderablesModel::GetInstance().AddRenderable(p_renderable.get(), name);
            g_renderable_points.emplace_back(std::move(p_renderable));
        }
    }

    void _DoToggleViewMode()
    {
        auto p_current_renderable = Actions::Configuration::GetCurrentRenderable();
        if (!p_current_renderable)
            return;

        if (auto p_renderable_mesh = qobject_cast<Rendering::RenderableMesh*>(p_current_renderable))
        {
            auto current_style = p_renderable_mesh->GetRenderingStyle();
            if (current_style == Rendering::RenderableMesh::RenderingStyle::Opaque)
                p_renderable_mesh->SetRenderingStyle(Rendering::RenderableMesh::RenderingStyle::Transparent);
            else if (current_style == Rendering::RenderableMesh::RenderingStyle::Transparent)
                p_renderable_mesh->SetRenderingStyle(Rendering::RenderableMesh::RenderingStyle::Opaque);
            else
                Q_ASSERT(false);

            return;
        }

        if (auto p_renderable_box = qobject_cast<Rendering::RenderableBox*>(p_current_renderable))
        {
            auto current_style = p_renderable_box->GetRenderingStyle();
            if (current_style == Rendering::RenderableBox::RenderingStyle::Opaque)
                p_renderable_box->SetRenderingStyle(Rendering::RenderableBox::RenderingStyle::Transparent);
            else if (current_style == Rendering::RenderableBox::RenderingStyle::Transparent)
                p_renderable_box->SetRenderingStyle(Rendering::RenderableBox::RenderingStyle::Opaque);
            else
                Q_ASSERT(false);

            return;
        }

        if (auto p_renderable_grid = qobject_cast<Rendering::RenderableVoxelGrid*>(p_current_renderable))
        {
            auto current_style = p_renderable_grid->GetRenderingStyle();
            if (current_style == Rendering::RenderableVoxelGrid::RenderingStyle::Opaque)
                p_renderable_grid->SetRenderingStyle(Rendering::RenderableVoxelGrid::RenderingStyle::Transparent);
            else if (current_style == Rendering::RenderableVoxelGrid::RenderingStyle::Transparent)
                p_renderable_grid->SetRenderingStyle(Rendering::RenderableVoxelGrid::RenderingStyle::Opaque);
            else
                Q_ASSERT(false);

            return;
        }
    }

    void _DoShowHide()
    {
        auto p_current_renderable = Actions::Configuration::GetCurrentRenderable();
        if (!p_current_renderable)
            return;

        auto& model = RenderablesModel::GetInstance();
        auto indexes = model.match(model.index(0), RenderablesModel::RawRenderablePtr, QVariant::fromValue(p_current_renderable));
        if (indexes.empty())
            return;
        Q_ASSERT(indexes.size() == 1);
        bool is_visible = indexes.at(0).data(RenderablesModel::Visibility).toBool();
        model.SetRenderableVisible(p_current_renderable, !is_visible);
    }

    void _DoTranslate()
    {
        auto p_current_renderable = Actions::Configuration::GetCurrentRenderable();
        if (!p_current_renderable)
            return;

        UI::TranslateParameters params;
        UI::TranslateDialog dialog(params);
        if (dialog.exec() == QDialog::Accepted)
        {
            TransformMatrix transform;
            transform.Translate(params.m_translate_x, params.m_translate_y, params.m_translate_z);
            transform.Rotate(params.m_rotate_x, Vector3D(1, 0, 0));
            transform.Rotate(params.m_rotate_y, Vector3D(0, 1, 0));
            transform.Rotate(params.m_rotate_z, Vector3D(0, 0, 1));
            transform.Scale(params.m_scale);
            p_current_renderable->Transform(transform);
        }
    }

    void _DoLocalize()
    {
        static bool nested_function_call_blocker_hack = false;
        if (nested_function_call_blocker_hack)
            return;
        nested_function_call_blocker_hack = true;

        auto p_dialog = std::make_unique<UI::LocalizeDialog>();
        bool result = QObject::connect(p_dialog.get(), &QObject::destroyed, [] { nested_function_call_blocker_hack = false; });
        Q_ASSERT(result);
        Q_UNUSED(result);

        p_dialog->setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose, true);
        p_dialog->setWindowFlag(Qt::WindowType::WindowMinimizeButtonHint, false);
        p_dialog->setWindowFlag(Qt::WindowType::WindowContextHelpButtonHint, false);
        p_dialog.release()->show();
    }

    void _DoSubdivide()
    {
        auto p_current_renderable = Actions::Configuration::GetCurrentRenderable();
        if (!p_current_renderable)
            return;
        
        auto p_mesh_renderable = qobject_cast<Rendering::RenderableMesh*>(p_current_renderable);
        if (!p_mesh_renderable)
            return;

        auto p_mesh = p_mesh_renderable->GetMesh();
        if (!p_mesh)
            return;

        UI::SubdivideParameters dialog_params;
        UI::SubdivideDialog dialog(dialog_params);
        if (dialog.exec() != QDialog::Accepted)
            return;

        auto subdivider = [&]
        {
            SQRT3MeshSubdivider::Params params;
            params.m_edge_length_threshold = dialog_params.m_edge_length_threshold;
            params.m_apply_smoothing = dialog_params.m_appy_smoothing;

            SQRT3MeshSubdivider sqrt3;
            sqrt3.SetParams(params);
            sqrt3.Subdivide(*p_mesh);
        };
        UI::RunInThread(subdivider, "Subdivision");

        // notify geometry changed
        emit p_current_renderable->RenderableRendererChanged();
    }
}

namespace Actions
{
    namespace Configuration
    {
        void SetCurrentRenderable(Rendering::IRenderable* ip_renderable)
        {
            g_current_renderable = ip_renderable;
        }

        Rendering::IRenderable* GetCurrentRenderable()
        {
            return g_current_renderable;
        }
    }

    QAction* GetLoadMeshAction()
    {
        static std::unique_ptr<QAction> mp_action;
        if (!mp_action)
        {
            mp_action = std::make_unique<QAction>();
            mp_action->setText("Load Mesh");
            bool is_connected = QObject::connect(mp_action.get(), &QAction::triggered, &_DoLoadMesh);
            Q_ASSERT(is_connected);
            Q_UNUSED(is_connected);
        }
        return mp_action.get();
    }

    QAction* GetAddPointAction()
    {
        static std::unique_ptr<QAction> mp_action;
        if (!mp_action)
        {
            mp_action = std::make_unique<QAction>();
            mp_action->setText("Add Point");
            bool is_connected = QObject::connect(mp_action.get(), &QAction::triggered, &_DoAddPoint);
            Q_ASSERT(is_connected);
            Q_UNUSED(is_connected);
        }
        return mp_action.get();
    }


    QAction* GetRemoveSelectedAction()
    {
        static std::unique_ptr<QAction> mp_action;
        if (!mp_action)
        {
            mp_action = std::make_unique<QAction>();
            mp_action->setText("Remove Selected");
            bool is_connected = QObject::connect(mp_action.get(), &QAction::triggered, &_DoRemoveSelected);
            Q_ASSERT(is_connected);
            Q_UNUSED(is_connected);
        }
        return mp_action.get();
    }

    QAction* GetHideShowSelectedAction()
    {
        static std::unique_ptr<QAction> mp_action;
        if (!mp_action)
        {
            mp_action = std::make_unique<QAction>();
            mp_action->setText("Show/Hide Selected");
            bool is_connected = QObject::connect(mp_action.get(), &QAction::triggered, &_DoShowHide);
            Q_ASSERT(is_connected);
            Q_UNUSED(is_connected);
        }
        return mp_action.get();
    }

    QAction* GetVoxelizeAction()
    {
        static std::unique_ptr<QAction> mp_action;
        if (!mp_action)
        {
            mp_action = std::make_unique<QAction>();
            mp_action->setText("Voxelize Selected");
            bool is_connected = QObject::connect(mp_action.get(), &QAction::triggered, &_DoVoxelize);
            Q_ASSERT(is_connected);
            Q_UNUSED(is_connected);
        }
        return mp_action.get();
    }

    QAction* GetToggleViewModeAction()
    {
        static std::unique_ptr<QAction> mp_action;
        if (!mp_action)
        {
            mp_action = std::make_unique<QAction>();
            mp_action->setText("Toggle View Mode");
            bool is_connected = QObject::connect(mp_action.get(), &QAction::triggered, &_DoToggleViewMode);
            Q_ASSERT(is_connected);
            Q_UNUSED(is_connected);
        }
        return mp_action.get();
    }

    QAction* GetTranslateAction()
    {
        static std::unique_ptr<QAction> mp_action;
        if (!mp_action)
        {
            mp_action = std::make_unique<QAction>();
            mp_action->setText("Translate");
            bool is_connected = QObject::connect(mp_action.get(), &QAction::triggered, &_DoTranslate);
            Q_ASSERT(is_connected);
            Q_UNUSED(is_connected);
        }
        return mp_action.get();
    }

    QAction* GetLocalizeAction()
    {
        static std::unique_ptr<QAction> mp_action;
        if (!mp_action)
        {
            mp_action = std::make_unique<QAction>();
            mp_action->setText("Localize");
            bool is_connected = QObject::connect(mp_action.get(), &QAction::triggered, &_DoLocalize);
            Q_ASSERT(is_connected);
            Q_UNUSED(is_connected);
        }
        return mp_action.get();
    }

    QAction* GetSubdivideAction()
    {
        static std::unique_ptr<QAction> mp_action;
        if (!mp_action)
        {
            mp_action = std::make_unique<QAction>();
            mp_action->setText("Subdivide");
            bool is_connected = QObject::connect(mp_action.get(), &QAction::triggered, &_DoSubdivide);
            Q_ASSERT(is_connected);
            Q_UNUSED(is_connected);
        }
        return mp_action.get();
    }

    void RunLoadForMesh(const QString& i_file)
    {
        _DoLoadInThreadImpl({ i_file });
    }
}
