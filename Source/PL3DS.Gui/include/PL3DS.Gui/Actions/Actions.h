#pragma once

class QAction;
class QString;

namespace Rendering
{
    class IRenderable;
}

namespace Actions
{
    namespace Configuration
    {
        void                    SetCurrentRenderable(Rendering::IRenderable* ip_renderable);
        Rendering::IRenderable* GetCurrentRenderable();
    }


    QAction* GetLoadMeshAction();
    QAction* GetAddPointAction();
    QAction* GetRemoveSelectedAction();
    QAction* GetHideShowSelectedAction();
    QAction* GetVoxelizeAction();
    QAction* GetToggleViewModeAction();
    QAction* GetTranslateAction();


    void RunLoadForMesh(const QString& i_file);
    
}
