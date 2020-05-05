#pragma once

#include <QAbstractListModel>

#include <boost/optional.hpp>

#include <memory>

namespace Rendering
{
    class IRenderable;
}

class RenderablesModel : public QAbstractListModel
{
    Q_OBJECT
    Q_DISABLE_COPY(RenderablesModel)

public:
    enum
    {
        RawRenderablePtr = Qt::UserRole + 1,
        Visibility       = Qt::UserRole + 2,
        Color            = Qt::UserRole + 3,
    };

    static RenderablesModel& GetInstance();

    int rowCount(const QModelIndex& i_parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& i_index, int i_role = Qt::DisplayRole) const override;

    void AddRenderable(Rendering::IRenderable* ip_renderable, const QString& i_name_hint, bool i_visible = true);
    void RemoveRenderable(Rendering::IRenderable* ip_renderable);
    void SetRenderableVisible(Rendering::IRenderable* ip_renderable, bool i_visible);

private:
    RenderablesModel();

    boost::optional<int> _FindRenderable(Rendering::IRenderable* ip_renderable) const;
    
    struct Impl;
    std::unique_ptr<Impl> mp_impl;
};
