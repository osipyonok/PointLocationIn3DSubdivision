#include "PL3DS.Gui/Utilities/MeshesProxyModel.h"

#include "PL3DS.Gui/RenderablesModel.h"

#include <Rendering.Core/RenderableMesh.h>

MeshesProxyModel::MeshesProxyModel(QObject* ip_parent/* = nullptr*/)
    : QSortFilterProxyModel(ip_parent)
{
    setSourceModel(&RenderablesModel::GetInstance());

    bool is_connected = false;
    is_connected = connect(sourceModel(), &QAbstractItemModel::rowsAboutToBeRemoved, this, [this](const QModelIndex&, int i_first, int i_last)
    {
        for (int row = i_first; row <= i_last; ++row)
        {
            auto it = m_checked_states.find(row);
            if (it != m_checked_states.end())
            {
                m_checked_states.erase(it);
            }
        }
    });
    Q_ASSERT(is_connected);

    Q_UNUSED(is_connected);
}

bool MeshesProxyModel::filterAcceptsRow(int i_source_row, const QModelIndex& i_source_parent) const
{
    auto index = sourceModel()->index(i_source_row, 0, i_source_parent);
    if (!index.isValid())
        return false;

    auto renderable_as_variant = index.data(RenderablesModel::RawRenderablePtr);
    if (!renderable_as_variant.isValid())
        return false;

    auto p_renderable = renderable_as_variant.value<Rendering::IRenderable*>();
    if (!p_renderable)
        return false;

    auto p_renderable_mesh = qobject_cast<Rendering::RenderableMesh*>(p_renderable);
    if (!p_renderable_mesh)
        return false;

    return true;
}

Qt::ItemFlags MeshesProxyModel::flags(const QModelIndex& i_index) const
{
    return Qt::ItemIsUserCheckable | QSortFilterProxyModel::flags(i_index);
}

QVariant MeshesProxyModel::data(const QModelIndex& i_index, int i_role) const
{
    if (!i_index.isValid())
        return QVariant::Invalid;

    if (i_role != Qt::CheckStateRole)
        return QSortFilterProxyModel::data(i_index, i_role);

    auto row = i_index.row();
    auto it = m_checked_states.find(row);
    if (m_checked_states.find(row) == m_checked_states.end())
    {
        return Qt::Checked;
    }

    return it->second;
}

bool MeshesProxyModel::setData(const QModelIndex& i_index, const QVariant& i_value, int i_role)
{
    if (!i_index.isValid())
        return QVariant::Invalid;

    if (i_role != Qt::CheckStateRole)
        return QSortFilterProxyModel::setData(i_index, i_value, i_role);

    auto row = i_index.row();
    auto state = i_value.value<Qt::CheckState>();

    if (m_checked_states[row] == state)
        return true;

    m_checked_states[row] = state;
    emit dataChanged(i_index, i_index, { Qt::CheckStateRole });

    return true;
}
