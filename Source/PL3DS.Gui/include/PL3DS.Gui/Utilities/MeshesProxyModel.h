#include <QSortFilterProxyModel>

#include <unordered_map>

class MeshesProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    MeshesProxyModel(QObject* ip_parent = nullptr);

    bool filterAcceptsRow(int i_source_row, const QModelIndex& i_source_parent) const override;
    Qt::ItemFlags flags(const QModelIndex& i_index) const override;

    QVariant data(const QModelIndex& i_index, int i_role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& i_index, const QVariant& i_value, int i_role = Qt::EditRole) override;

private:

    std::unordered_map<int, Qt::CheckState> m_checked_states;
};
