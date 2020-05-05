#include "PL3DS.Gui/Dialogs/LocalizeDialog.h"
#include "ui_LocalizeDialog.h"

#include "PL3DS.Gui/RenderablesModel.h"
#include "PL3DS.Gui/Utilities/GeneralUtilities.h"
#include "PL3DS.Gui/Utilities/MeshesProxyModel.h"
#include "PL3DS.Gui/Utilities/ProgressDialog.h"

#include <Math.Core/CommonUtilities.h>
#include <Math.Core/Mesh.h>
#include <Math.Core/MeshTriangle.h>
#include <Math.Core/TransformMatrix.h>

#include <Math.Algos/PointLocalizerVoxelized.h>

#include <Math.DataStructures/TrianglesOctree.h>
#include <Math.DataStructures/TrianglesTree.h>
#include <Math.DataStructures/VoxelGrid.h>

#include <Rendering.Core/RenderableMesh.h>
#include <Rendering.Core/RenderablePoint.h>
#include <Rendering.Core/RenderableSegment.h>
#include <Rendering.Core/RenderableVoxelGrid.h>
#include <Rendering.Core/RenderableTrianglesTree.h>

#include <Rendering.Main/RenderablesController.h>

#include <Qt3DCore/QTransform>

#include <QString>
#include <QStringView>

#include <map>

namespace
{
    constexpr auto WINDOWS_ENDL = "\r\n";

    auto _GetMeshesWithTransformation(QAbstractItemModel* ip_model)
    {
        std::vector<std::pair<Mesh*, TransformMatrix>> meshes;
        for (int i = 0; i < ip_model->rowCount(); ++i)
        {
            auto index = ip_model->index(i, 0);

            if (index.data(Qt::CheckStateRole).value<Qt::CheckState>() != Qt::Checked)
                continue;

            auto renderable = index.data(RenderablesModel::RawRenderablePtr);
            if (!renderable.isValid())
                continue;

            auto p_renderable = qobject_cast<Rendering::RenderableMesh*>(renderable.value<Rendering::IRenderable*>());
            if (!p_renderable)
                continue;

            meshes.emplace_back(p_renderable->GetMesh(), p_renderable->GetTransform());
        }

        return std::move(meshes);
    }
}

namespace UI
{
    namespace Details
    {

        PointPreviewRenderable::PointPreviewRenderable()
        {
            // point
            auto p_renderable_point = std::make_unique<Rendering::RenderablePoint>(m_position);
            p_renderable_point->SetColor(GetColor());
            m_renderables.emplace_back(std::move(p_renderable_point));

            constexpr auto inf = 200000.;
            auto p_segment_x_axis = std::make_unique<Rendering::RenderableSegment>(Point3D{ -inf, 0., 0. }, Point3D{ inf, 0., 0. });
            auto p_segment_y_axis = std::make_unique<Rendering::RenderableSegment>(Point3D{ 0., -inf, 0. }, Point3D{ 0., inf, 0. });
            auto p_segment_z_axis = std::make_unique<Rendering::RenderableSegment>(Point3D{ 0., 0., -inf }, Point3D{ 0., 0., inf });

            p_segment_x_axis->SetColor(GetColor());
            p_segment_y_axis->SetColor(GetColor());
            p_segment_z_axis->SetColor(GetColor());

            m_renderables.emplace_back(std::move(p_segment_x_axis));
            m_renderables.emplace_back(std::move(p_segment_y_axis));
            m_renderables.emplace_back(std::move(p_segment_z_axis));
        }

        PointPreviewRenderable::~PointPreviewRenderable()
        {
            _OnDestructed();
        }

        std::unique_ptr<Qt3DCore::QComponent> PointPreviewRenderable::GetMaterial() const
        {
            return nullptr;
        }

        std::unique_ptr<Qt3DCore::QTransform> PointPreviewRenderable::GetTransformation() const
        {
            return nullptr;
        }

        std::unique_ptr<Qt3DCore::QComponent> PointPreviewRenderable::GetRenderer() const
        {
            return nullptr;
        }

        void PointPreviewRenderable::SetColor(const QColor& i_color)
        {
            assert(false);
        }

        QColor PointPreviewRenderable::GetColor() const
        {
            return QColor("grey");
        }

        const TransformMatrix& PointPreviewRenderable::GetTransform() const
        {
            assert(false); 
            return m_renderables.at(0)->GetTransform();
        }

        void PointPreviewRenderable::Transform(const TransformMatrix & i_transform)
        {
            for (const auto& p_renderable : m_renderables)
                p_renderable->Transform(i_transform);

            emit RenderableTransformationChanged();
        }

        std::vector<Rendering::IRenderable*> PointPreviewRenderable::GetNestedRenderables() const
        {
            std::vector<IRenderable*> result;
            for (const auto& p_renderables : m_renderables)
                result.push_back(p_renderables.get());
            return std::move(result);
        }

        void PointPreviewRenderable::SetPoint(const Point3D & i_point)
        {
            if (i_point == m_position)
                return;

            auto diff = i_point - m_position;
            TransformMatrix matrix;
            matrix.Translate(diff.GetX(), diff.GetY(), diff.GetZ());
            m_position = i_point;
            Transform(matrix);
        }

    }

    struct LocalizeDialog::Impl
    {
        std::unique_ptr<Ui::LocalizeDialog> mp_ui = std::make_unique<Ui::LocalizeDialog>();
        std::unique_ptr<::UI::Details::PointPreviewRenderable> mp_preview = std::make_unique< ::UI::Details::PointPreviewRenderable>();

        // voxel based stuff
        std::unique_ptr<PointLocalizerVoxelized> mp_localizer_voxelized = std::make_unique<PointLocalizerVoxelized>();
        std::unique_ptr<Rendering::RenderableVoxelGrid> mp_renderable_voxel_grid;
        std::map<size_t, QString> m_index_to_name_map;

        // kd tree based stuff
        std::unique_ptr<TrianglesTree> mp_kd_tree;
        std::list<Triangle> m_kd_transformed_triangles;
        std::unordered_map<Triangle*, QStringView> m_kd_triangle_to_mesh_map;
        std::unique_ptr<Rendering::RenderableTrianglesTree> mp_renderable_kd_tree;

        // octree based stuff
        std::unique_ptr<TrianglesOcTree> mp_octree;
        std::list<Triangle> m_oct_transformed_triangles;
        std::unique_ptr<Rendering::RenderableTrianglesTree> mp_renderable_octree;
    };

    LocalizeDialog::LocalizeDialog(QDialog* ip_parent)
        : QDialog(ip_parent)
        , mp_impl(std::make_unique<LocalizeDialog::Impl>())
    {
        Ui::LocalizeDialog& ui = *mp_impl->mp_ui;
        ui.setupUi(this);

        auto p_meshes_model = new MeshesProxyModel(this);
        ui.mp_list_meshes->setModel(p_meshes_model);

        mp_impl->mp_preview->SetPoint({ ui.mp_spin_x->value(), ui.mp_spin_y->value(), ui.mp_spin_z->value() });

        bool is_connected = false;

        auto update_preview = [this]
        {
            Point3D point(mp_impl->mp_ui->mp_spin_x->value(),
                          mp_impl->mp_ui->mp_spin_y->value(),
                          mp_impl->mp_ui->mp_spin_z->value());
            mp_impl->mp_preview->SetPoint(point);
        };

        is_connected = connect(ui.mp_spin_x, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, update_preview);
        Q_ASSERT(is_connected);
        is_connected = connect(ui.mp_spin_y, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, update_preview);
        Q_ASSERT(is_connected);
        is_connected = connect(ui.mp_spin_z, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, update_preview);
        Q_ASSERT(is_connected);

        auto invalidate = [this]
        {
            _UpdateSliders();

            mp_impl->mp_localizer_voxelized.reset();
            mp_impl->mp_renderable_voxel_grid.reset();

            mp_impl->mp_kd_tree.reset();
            mp_impl->m_kd_triangle_to_mesh_map.clear();
            mp_impl->m_kd_transformed_triangles.clear();
            mp_impl->mp_renderable_kd_tree.reset();

            mp_impl->mp_octree.reset();
            mp_impl->m_oct_transformed_triangles.clear();
            mp_impl->mp_renderable_octree.reset();
        };

        is_connected = connect(p_meshes_model, &QAbstractItemModel::rowsAboutToBeRemoved, this, [=](const QModelIndex&, int i_first, int i_last)
        {
            bool changed = false;

            for (int i = i_first; i <= i_last; ++i)
            {
                auto index = p_meshes_model->index(i_first, 0);
                auto data = index.data(Qt::CheckStateRole);
                if (data.isValid() && data.value<Qt::CheckState>() == Qt::Checked)
                {
                    changed = true;
                }
            }

            if (!changed)
                return;

            invalidate();
        });
        Q_ASSERT(is_connected);

        is_connected = connect(p_meshes_model, &QAbstractItemModel::modelAboutToBeReset, this, invalidate);
        Q_ASSERT(is_connected);

        _InitVoxelBased();
        _InitKDTreeBased();
        _InitOcTreeBased();

        _UpdateSliders();

        Rendering::RenderablesController::GetInstance().AddRenderable(*mp_impl->mp_preview);

        Q_UNUSED(is_connected);
    }

    LocalizeDialog::~LocalizeDialog()
    {
        Rendering::RenderablesController::GetInstance().RemoveRenderable(mp_impl->mp_preview.get());
    }

    void LocalizeDialog::_InitVoxelBased()
    {
        Ui::LocalizeDialog& ui = *mp_impl->mp_ui;
        auto p_meshes_model = ui.mp_list_meshes->model();

        bool is_connected = false;

        is_connected = connect(ui.mp_btn_voxel_build, &QAbstractButton::clicked, this, [=]
        {
            mp_impl->mp_renderable_voxel_grid.reset();
            mp_impl->mp_localizer_voxelized = std::make_unique<PointLocalizerVoxelized>();

            auto meshes = _GetMeshesWithTransformation(p_meshes_model);

            std::vector<size_t> indexes;
            indexes.reserve(meshes.size());
            Utilities::TimeMemoryLogger logger;
            auto builder = [this, &meshes, &logger, &indexes]
            {
                PointLocalizerVoxelized::Params params;
                params.m_voxel_size_x = mp_impl->mp_ui->mp_spin_voxel_x->value();
                params.m_voxel_size_y = mp_impl->mp_ui->mp_spin_voxel_y->value();
                params.m_voxel_size_z = mp_impl->mp_ui->mp_spin_voxel_z->value();

                logger.Start();
                for (const auto& mesh : meshes)
                {
                    auto mesh_id = mp_impl->mp_localizer_voxelized->AddMesh(*mesh.first, mesh.second);
                    indexes.emplace_back(mesh_id);
                }
                mp_impl->mp_localizer_voxelized->Build(params);
                logger.Stop();
            };
            UI::RunInThread(builder, "Building voxelization");

            _LogMessage(QString("Build of voxelization finished, elapsed time: %1 sec.").arg(QString::number(logger.GetElapsedTimeSec(), 'f')));
            _LogMessage(QString("Memory difference is: %1 mb.").arg(QString::number(logger.GetMemoryDifferenceMb(), 'f')));

            mp_impl->m_index_to_name_map.clear();
            for (size_t i = 0; i < meshes.size(); ++i)
            {
                mp_impl->m_index_to_name_map[indexes[i]] = meshes[i].first->GetName();
            }

            mp_impl->mp_renderable_voxel_grid = std::make_unique<Rendering::RenderableVoxelGrid>(*mp_impl->mp_localizer_voxelized->GetCachedGrid().lock());
            RenderablesModel::GetInstance().AddRenderable(mp_impl->mp_renderable_voxel_grid.get(), "Voxelization");
        });
        Q_ASSERT(is_connected);

        is_connected = connect(ui.mp_btn_voxel_localize, &QAbstractButton::clicked, this, [=]
        {
            Point3D point(mp_impl->mp_ui->mp_spin_x->value(),
                          mp_impl->mp_ui->mp_spin_y->value(),
                          mp_impl->mp_ui->mp_spin_z->value());

            Utilities::TimeMemoryLogger logger;
            PointLocalizerVoxelized::ReturnCode return_code;
            size_t mesh_id;
            auto localizer = [this, &point, &logger, &return_code, &mesh_id]
            {
                logger.Start();

                mesh_id = mp_impl->mp_localizer_voxelized->Localize(point, &return_code);

                logger.Stop();
            };
            UI::RunInThread(localizer, "Localizing point");

            switch (return_code)
            {
            case PointLocalizerVoxelized::ReturnCode::Ok:
                if (mesh_id == std::numeric_limits<size_t>::max())
                    _LogMessage("Point is located outside of all meshes");
                else
                    _LogMessage(QString("Point is located at mesh with name: %1").arg(mp_impl->m_index_to_name_map[mesh_id]));

                _LogMessage(QString("Elapsed time: %1 sec.").arg(QString::number(logger.GetElapsedTimeSec(), 'f')));
                break;

            case PointLocalizerVoxelized::ReturnCode::VoxelizationWasNotBuild:
                _LogMessage("Error, voxelization was not built");
                break;
            default:
                assert(false);
                break;
            }

        });
        Q_ASSERT(is_connected);

        is_connected = connect(ui.mp_btn_toggle_voxelization, &QAbstractButton::clicked, this, [=]
        {
            if (!mp_impl->mp_renderable_voxel_grid)
                return;

            auto current = mp_impl->mp_renderable_voxel_grid->GetRenderingStyle();
            if (current == Rendering::RenderableVoxelGrid::RenderingStyle::Opaque)
                mp_impl->mp_renderable_voxel_grid->SetRenderingStyle(Rendering::RenderableVoxelGrid::RenderingStyle::Transparent);
            else if (current == Rendering::RenderableVoxelGrid::RenderingStyle::Transparent)
                mp_impl->mp_renderable_voxel_grid->SetRenderingStyle(Rendering::RenderableVoxelGrid::RenderingStyle::Opaque);
            else
                Q_ASSERT(false);
        });
        Q_ASSERT(is_connected);

        is_connected = connect(ui.mp_btn_show_hide_voxelization, &QAbstractButton::clicked, this, [=]
        {
            if (!mp_impl->mp_renderable_voxel_grid)
                return;

            auto& model = RenderablesModel::GetInstance();
            auto indexes = model.match(model.index(0, 0), RenderablesModel::RawRenderablePtr, QVariant::fromValue<Rendering::IRenderable*>(mp_impl->mp_renderable_voxel_grid.get()), 1, Qt::MatchExactly);
            if (indexes.empty())
                return;

            Q_ASSERT(indexes.size() == 1);
            auto index = indexes.at(0);
            bool is_visible = index.data(RenderablesModel::Visibility).toBool();
            model.SetRenderableVisible(mp_impl->mp_renderable_voxel_grid.get(), !is_visible);
        });
        Q_ASSERT(is_connected);

        Q_UNUSED(is_connected);
    }

    void LocalizeDialog::_InitKDTreeBased()
    {
        bool is_connected = false;

        is_connected = connect(mp_impl->mp_ui->mp_btn_kd_build, &QAbstractButton::clicked, this, [=]
        {
            mp_impl->mp_renderable_kd_tree.reset();
            mp_impl->mp_kd_tree = std::make_unique<TrianglesTree>();
            mp_impl->m_kd_triangle_to_mesh_map.clear();
            mp_impl->m_kd_transformed_triangles.clear();

            Utilities::TimeMemoryLogger logger;
            auto builder = [&]
            {
                auto meshes = _GetMeshesWithTransformation(mp_impl->mp_ui->mp_list_meshes->model());

                std::vector<Triangle*> triangles;
                for (const auto& mesh_transform : meshes)
                {
                    const auto p_mesh = mesh_transform.first;
                    const auto& transform = mesh_transform.second;

                    for (size_t i = 0; i < p_mesh->GetTrianglesCount(); ++i)
                    {
                        if (auto p_triangle = p_mesh->GetTriangle(i).lock())
                        {
                            auto point1 = p_triangle->GetPoint(0);
                            auto point2 = p_triangle->GetPoint(1);
                            auto point3 = p_triangle->GetPoint(2);
                            transform.ApplyTransformation(point1);
                            transform.ApplyTransformation(point2);
                            transform.ApplyTransformation(point3);

                            mp_impl->m_kd_transformed_triangles.emplace_back(point1, point2, point3);
                            mp_impl->m_kd_triangle_to_mesh_map[&mp_impl->m_kd_transformed_triangles.back()] = QStringView(p_mesh->GetName());
                            triangles.emplace_back(&mp_impl->m_kd_transformed_triangles.back());
                        }
                        else
                        {
                            assert(false);
                        }
                    }
                }

                logger.Start();
                mp_impl->mp_kd_tree->Build(triangles);
                logger.Stop();
            };

            UI::RunInThread(builder, "Build K-d Tree");

            _LogMessage(QString("Build of k-d tree finished, elapsed time: %1 sec.").arg(QString::number(logger.GetElapsedTimeSec(), 'f')));
            _LogMessage(QString("Memory difference is: %1 mb.").arg(QString::number(logger.GetMemoryDifferenceMb(), 'f')));

            auto p_tree_source = std::make_unique<Rendering::TrianglesTreeDataSource>(*mp_impl->mp_kd_tree);
            mp_impl->mp_renderable_kd_tree = std::make_unique<Rendering::RenderableTrianglesTree>(std::move(p_tree_source));
            RenderablesModel::GetInstance().AddRenderable(mp_impl->mp_renderable_kd_tree.get(), "K-d tree");
            
            _UpdateSliders();
        });
        Q_ASSERT(is_connected);

        is_connected = connect(mp_impl->mp_ui->mp_btn_kd_localize, &QAbstractButton::clicked, this, [=]
        {
            if (!mp_impl->mp_kd_tree || !mp_impl->mp_kd_tree->WasBuild())
            {
                _LogMessage("Error, k-d tree was not built");
                return;
            }

            Point3D point(mp_impl->mp_ui->mp_spin_x->value(),
                          mp_impl->mp_ui->mp_spin_y->value(),
                          mp_impl->mp_ui->mp_spin_z->value());

            Utilities::TimeMemoryLogger logger;
            Triangle* p_triangle = nullptr;
            auto localizer = [&]
            {
                logger.Start();
                mp_impl->mp_kd_tree->Query(p_triangle, point);
                logger.Stop();
            };

            UI::RunInThread(localizer, "Localizing point");

            bool located_below = false;
            if (p_triangle)
            {
                auto loc_result = GetPointTriangleRelativeLocation(*p_triangle, point);
                located_below = loc_result == PointTriangleRelativeLocationResult::Below
                             || loc_result == PointTriangleRelativeLocationResult::OnSamePlane;
            }

            if (p_triangle && located_below)
            {
                _LogMessage(QString("Point is located at mesh with name: %1").arg(mp_impl->m_kd_triangle_to_mesh_map[p_triangle]));
            }
            else
            {
                _LogMessage("Point is located outside of all meshes");
            }
            _LogMessage(QString("Elapsed time: %1 sec.").arg(QString::number(logger.GetElapsedTimeSec(), 'f')));
        });
        Q_ASSERT(is_connected);

        is_connected = connect(mp_impl->mp_ui->mp_slider_kd_layer, &QSlider::valueChanged, this, [this](int i_value)
        {
            if (mp_impl->mp_renderable_kd_tree)
                mp_impl->mp_renderable_kd_tree->SetCurrentLayer(i_value);
        });
        Q_ASSERT(is_connected);

        is_connected = connect(mp_impl->mp_ui->mp_btn_kd_toggle, &QAbstractButton::clicked, this, [this]
        {
            if (!mp_impl->mp_renderable_kd_tree)
                return;

            auto current = mp_impl->mp_renderable_kd_tree->GetRenderingStyle();
            if (current == Rendering::RenderableTrianglesTree::RenderingStyle::Opaque)
                mp_impl->mp_renderable_kd_tree->SetRenderingStyle(Rendering::RenderableTrianglesTree::RenderingStyle::Transparent);
            else
                mp_impl->mp_renderable_kd_tree->SetRenderingStyle(Rendering::RenderableTrianglesTree::RenderingStyle::Opaque);
        });
        Q_ASSERT(is_connected);

        is_connected = connect(mp_impl->mp_ui->mp_btn_kd_show_hide, &QAbstractButton::clicked, this, [this]
        {
            if (!mp_impl->mp_renderable_kd_tree)
                return;

            auto& model = RenderablesModel::GetInstance();
            auto indexes = model.match(model.index(0, 0), RenderablesModel::RawRenderablePtr, QVariant::fromValue<Rendering::IRenderable*>(mp_impl->mp_renderable_kd_tree.get()), 1, Qt::MatchExactly);
            if (indexes.empty())
                return;

            Q_ASSERT(indexes.size() == 1);
            auto index = indexes.at(0);
            bool is_visible = index.data(RenderablesModel::Visibility).toBool();
            model.SetRenderableVisible(mp_impl->mp_renderable_kd_tree.get(), !is_visible);
        });
        Q_ASSERT(is_connected);

        Q_UNUSED(is_connected);
    }

    void LocalizeDialog::_InitOcTreeBased()
    {
        bool is_connected = false;

        is_connected = connect(mp_impl->mp_ui->mp_btn_octree_build, &QAbstractButton::clicked, this, [=]
        {
            mp_impl->mp_octree = std::make_unique<TrianglesOcTree>();

            Utilities::TimeMemoryLogger logger;
            auto builder = [&]
            {
                auto meshes = _GetMeshesWithTransformation(mp_impl->mp_ui->mp_list_meshes->model());

                std::vector<TriangleWithMeshTag> triangles;
                for (const auto& mesh_transform : meshes)
                {
                    const auto p_mesh = mesh_transform.first;
                    const auto& transform = mesh_transform.second;

                    for (size_t i = 0; i < p_mesh->GetTrianglesCount(); ++i)
                    {
                        if (auto p_triangle = p_mesh->GetTriangle(i).lock())
                        {
                            auto point1 = p_triangle->GetPoint(0);
                            auto point2 = p_triangle->GetPoint(1);
                            auto point3 = p_triangle->GetPoint(2);
                            transform.ApplyTransformation(point1);
                            transform.ApplyTransformation(point2);
                            transform.ApplyTransformation(point3);

                            mp_impl->m_oct_transformed_triangles.emplace_back(point1, point2, point3);
                            triangles.emplace_back(&mp_impl->m_oct_transformed_triangles.back(), QStringView(p_mesh->GetName()));
                        }
                        else
                        {
                            assert(false);
                        }
                    }
                }

                logger.Start();
                mp_impl->mp_octree->Build(triangles);
                logger.Stop();
            };

            UI::RunInThread(builder, "Build OcTree");

            _LogMessage(QString("Build of OcTree finished, elapsed time: %1 sec.").arg(QString::number(logger.GetElapsedTimeSec(), 'f')));
            _LogMessage(QString("Memory difference is: %1 mb.").arg(QString::number(logger.GetMemoryDifferenceMb(), 'f')));

            auto p_tree_source = std::make_unique<Rendering::TriangleOcTreeDataSource>(*mp_impl->mp_octree);
            mp_impl->mp_renderable_octree = std::make_unique<Rendering::RenderableTrianglesTree>(std::move(p_tree_source));
            RenderablesModel::GetInstance().AddRenderable(mp_impl->mp_renderable_octree.get(), "OcTree");

            _UpdateSliders();
        });
        Q_ASSERT(is_connected);

        is_connected = connect(mp_impl->mp_ui->mp_slider_octree_layer, &QSlider::valueChanged, this, [this](int i_value)
        {
            if (mp_impl->mp_renderable_octree)
                mp_impl->mp_renderable_octree->SetCurrentLayer(i_value);
        });
        Q_ASSERT(is_connected);

        is_connected = connect(mp_impl->mp_ui->mp_btn_octree_toggle, &QAbstractButton::clicked, this, [this]
        {
            if (!mp_impl->mp_renderable_octree)
                return;

            auto current = mp_impl->mp_renderable_octree->GetRenderingStyle();
            if (current == Rendering::RenderableTrianglesTree::RenderingStyle::Opaque)
                mp_impl->mp_renderable_octree->SetRenderingStyle(Rendering::RenderableTrianglesTree::RenderingStyle::Transparent);
            else
                mp_impl->mp_renderable_octree->SetRenderingStyle(Rendering::RenderableTrianglesTree::RenderingStyle::Opaque);
        });
        Q_ASSERT(is_connected);

        is_connected = connect(mp_impl->mp_ui->mp_btn_octree_show_hide, &QAbstractButton::clicked, this, [this]
        {
            if (!mp_impl->mp_renderable_octree)
                return;

            auto& model = RenderablesModel::GetInstance();
            auto indexes = model.match(model.index(0, 0), RenderablesModel::RawRenderablePtr, QVariant::fromValue<Rendering::IRenderable*>(mp_impl->mp_renderable_octree.get()), 1, Qt::MatchExactly);
            if (indexes.empty())
                return;

            Q_ASSERT(indexes.size() == 1);
            auto index = indexes.at(0);
            bool is_visible = index.data(RenderablesModel::Visibility).toBool();
            model.SetRenderableVisible(mp_impl->mp_renderable_octree.get(), !is_visible);
        });
        Q_ASSERT(is_connected);

        is_connected = connect(mp_impl->mp_ui->mp_btn_octree_localize, &QAbstractButton::clicked, this, [this]
        {
            if (!mp_impl->mp_octree || !mp_impl->mp_octree->WasBuild())
            {
                _LogMessage("Error, OcTree was not built");
                return;
            }

            Point3D point(mp_impl->mp_ui->mp_spin_x->value(),
                          mp_impl->mp_ui->mp_spin_y->value(),
                          mp_impl->mp_ui->mp_spin_z->value());

            Utilities::TimeMemoryLogger logger;
            TriangleOcTreeQueryResult query_result;
            auto localizer = [&]
            {
                logger.Start();
                mp_impl->mp_octree->Query(query_result, point);
                logger.Stop();
            };

            UI::RunInThread(localizer, "Localizing point");

            if (query_result.m_status == TriangleOcTreeQueryResult::Inside)
            {
                _LogMessage(QString("Point is located at mesh with name: %1").arg(query_result.m_mesh_name));
            }
            else
            {
                _LogMessage("Point is located outside of all meshes");
            }
            _LogMessage(QString("Elapsed time: %1 sec.").arg(QString::number(logger.GetElapsedTimeSec(), 'f')));
        });
        Q_ASSERT(is_connected);

        Q_UNUSED(is_connected);
    }

    void LocalizeDialog::_UpdateSliders()
    {
        // kd tree
        if (auto p_renderable_kd_tree = mp_impl->mp_renderable_kd_tree.get())
        {
            auto max = p_renderable_kd_tree->GetLayersCount();
            auto current = p_renderable_kd_tree->GetCurrentLayer();

            mp_impl->mp_ui->mp_slider_kd_layer->setEnabled(true);
            mp_impl->mp_ui->mp_slider_kd_layer->setMinimum(0);
            mp_impl->mp_ui->mp_slider_kd_layer->setMaximum(static_cast<int>(max));
            mp_impl->mp_ui->mp_slider_kd_layer->setValue(static_cast<int>(current));
        }
        else
        {
            mp_impl->mp_ui->mp_slider_kd_layer->setEnabled(false);
            mp_impl->mp_ui->mp_slider_kd_layer->setMinimum(0);
            mp_impl->mp_ui->mp_slider_kd_layer->setMaximum(0);
        }

        // octree
        if (auto p_renderable_octree = mp_impl->mp_renderable_octree.get())
        {
            auto max = p_renderable_octree->GetLayersCount();
            auto current = p_renderable_octree->GetCurrentLayer();

            mp_impl->mp_ui->mp_slider_octree_layer->setEnabled(true);
            mp_impl->mp_ui->mp_slider_octree_layer->setMinimum(0);
            mp_impl->mp_ui->mp_slider_octree_layer->setMaximum(static_cast<int>(max));
            mp_impl->mp_ui->mp_slider_octree_layer->setValue(static_cast<int>(current));
        }
        else
        {
            mp_impl->mp_ui->mp_slider_octree_layer->setEnabled(false);
            mp_impl->mp_ui->mp_slider_octree_layer->setMinimum(0);
            mp_impl->mp_ui->mp_slider_octree_layer->setMaximum(0);
        }
    }

    void LocalizeDialog::_LogMessage(const QString& i_str) const
    {
        mp_impl->mp_ui->mp_text_log->appendPlainText(i_str + WINDOWS_ENDL);
    }
}

