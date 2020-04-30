#pragma once

#include <QWidget>

#include <functional>

class QAction;
class RenderablesModel;

namespace Rendering
{
    class IRenderable;
}

namespace UI
{

    class MainWidget : public QWidget
    {
        Q_OBJECT

    public:
        struct Params
        {
            RenderablesModel* mp_renderables_model = nullptr;
            QAction* mp_load_mesh_action = nullptr;
            QAction* mp_add_point_action = nullptr;
            QAction* mp_remove_selected_action = nullptr;
            QAction* mp_toggle_visibility_action = nullptr;
            QAction* mp_toggle_view_mode_adction = nullptr;
            QAction* mp_voxelize_action = nullptr;
            QAction* mp_translate_action = nullptr;
            QAction* mp_localize_action = nullptr;

            std::function<void(Rendering::IRenderable*)> m_selected_renderable_setter;
            std::function<Rendering::IRenderable*(void)> m_selected_renderable_getter;
        };


        MainWidget(const Params& i_params, QWidget* ip_parent = nullptr);

        ~MainWidget() override;

    };

}
