#include "PL3DS.Gui/RenderablesModel.h"

#include <Rendering.Core/IRenderable.h>

#include <QBrush>
#include <QColor>

#include <vector>

namespace
{
    const auto EMPTY_RENDERABLE_NAME = QStringLiteral("Unnamed");

    struct MetaData
    {
        Rendering::IRenderable* mp_renedrable = nullptr;
        bool m_visible = true;
        QString m_name;
    };

}

struct RenderablesModel::Impl
{
    std::vector<MetaData> m_renderables;
};

int RenderablesModel::rowCount(const QModelIndex& i_parent) const
{
    return static_cast<int>(mp_impl->m_renderables.size());
}

QVariant RenderablesModel::data(const QModelIndex& i_index, int i_role) const
{
    const int row = i_index.row();
    if (row < 0 || row >= rowCount())
        return QVariant::Invalid;
    if (i_index.column() != 0)
    {
        assert(false);
        return QVariant::Invalid;
    }


    if (i_role == Qt::DisplayRole)
    {
        return mp_impl->m_renderables[row].m_name;
    }
    else if (i_role == Qt::ForegroundRole)
    {
        return QBrush(mp_impl->m_renderables[row].mp_renedrable->GetColor());
    }
    else if (i_role == RenderablesModel::RawRenderablePtr)
    {
        return QVariant::fromValue(mp_impl->m_renderables[row].mp_renedrable);
    }
    else if (i_role == RenderablesModel::Visibility)
    {
        return mp_impl->m_renderables[row].m_visible;
    }
    else if (i_role == RenderablesModel::Color)
    {
        return mp_impl->m_renderables[row].mp_renedrable->GetColor();
    }

    return QVariant::Invalid;
}

void RenderablesModel::AddRenderable(Rendering::IRenderable* ip_renderable, const QString& i_name, bool i_visible)
{
    MetaData meta;
    meta.mp_renedrable = ip_renderable;
    meta.m_name = i_name.isEmpty() ? EMPTY_RENDERABLE_NAME : i_name;
    meta.m_visible = i_visible;

    beginInsertRows({}, rowCount(), rowCount());
    mp_impl->m_renderables.emplace_back(meta);
    endInsertRows();

    bool is_connected = false;
    is_connected = connect(ip_renderable, &Rendering::IRenderable::RenderableDestructed, this, [this, ip_renderable]
    { 
        RemoveRenderable(ip_renderable); 
    });
    Q_ASSERT(is_connected);
    is_connected = connect(ip_renderable, &Rendering::IRenderable::RenderableMaterialChanged, this, [this, ip_renderable]
    {
        auto pos = _FindRenderable(ip_renderable);
        if (!pos.has_value())
            return;

        emit dataChanged(index(pos.get()), index(pos.get()), { Color, Qt::ForegroundRole });
    });
    Q_ASSERT(is_connected);


    Q_UNUSED(is_connected);
}

void RenderablesModel::RemoveRenderable(Rendering::IRenderable* ip_renderable)
{
    auto pos = _FindRenderable(ip_renderable);
    if (!pos.has_value())
        return;

    beginRemoveRows({}, pos.get(), pos.get());
    mp_impl->m_renderables.erase(mp_impl->m_renderables.begin() + pos.get());
    endRemoveRows();
}

void RenderablesModel::SetRenderableVisible(Rendering::IRenderable* ip_renderable, bool i_visible)
{
    auto pos = _FindRenderable(ip_renderable);
    if (!pos.has_value() || mp_impl->m_renderables[pos.get()].m_visible == i_visible)
        return;

    mp_impl->m_renderables[pos.get()].m_visible = i_visible;
    emit dataChanged(index(pos.get()), index(pos.get()), { Visibility });
}

RenderablesModel::RenderablesModel()
    : mp_impl(std::make_unique<Impl>())
{

}

boost::optional<int> RenderablesModel::_FindRenderable(Rendering::IRenderable * ip_renderable) const
{
    auto it = std::find_if(mp_impl->m_renderables.begin(), mp_impl->m_renderables.end(), [ip_renderable](const MetaData& i_meta)
    {
        return i_meta.mp_renedrable == ip_renderable;
    });

    if (it == mp_impl->m_renderables.end())
        return boost::optional<int>{};

    return static_cast<int>(std::distance(mp_impl->m_renderables.begin(), it));
}

RenderablesModel& RenderablesModel::GetInstance()
{
    static RenderablesModel instance;
    return instance;
}
