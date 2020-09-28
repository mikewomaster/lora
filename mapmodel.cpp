#include "mapmodel.h"
#include <QAbstractTableModel>

#define MAP_ID_COLUMN  0
#define MAP_REG_COLUMN 1

mapModel::mapModel(QObject *parent)
    : QAbstractTableModel(parent)
{

}

void mapModel::updateData(QList<mapModelData> recordList)
{
    m_recordList = recordList;
    beginResetModel();
    endResetModel();
}

void mapModel::clearDate()
{
    QList<mapModelData> recordList;
    for (int i = 1; i <= 20; ++i)
    {
        mapModelData record;
        record.id = 0;
        record.reg = 0;
        recordList.append(record);
    }
    updateData(recordList);
}

int mapModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_recordList.count();
}

int mapModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 2;
}

bool mapModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    int nColumn = index.column();
    mapModelData record = m_recordList.at(index.row());
    switch (role)
    {
        case Qt::EditRole:
        {
                if (nColumn == MAP_ID_COLUMN) {
                    record.id = value.toUInt();
                    m_recordList.replace(index.row(), record);
                    emit dataChanged(index, index);
                    return true;
                } else if (nColumn == MAP_REG_COLUMN) {
                    record.reg = value.toUInt();
                    m_recordList.replace(index.row(), record);
                    emit dataChanged(index, index);
                    return true;
                }
        }
        default:
            return false;
    }
    return false;
}

QVariant mapModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int nRow = index.row();
    int nColumn = index.column();
    mapModelData record = m_recordList.at(nRow);

    switch (role)
    {

    case Qt::TextAlignmentRole:
        return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
    case Qt::DisplayRole:
    {
        if (nColumn == MAP_ID_COLUMN)
            return record.id == 0 ? "" : QString::number(record.id);

        if (nColumn == MAP_REG_COLUMN)
            return record.reg == 0 ? "" : QString::number(record.reg);

        return "";
    }
    default:
        return QVariant();
    }

    return QVariant();
}

QVariant mapModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (role)
    {
        case Qt::TextAlignmentRole:
            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);

        case Qt::DisplayRole:
        {
            if (orientation == Qt::Horizontal)
            {
                if (section == MAP_ID_COLUMN)
                    return QStringLiteral("ID");

                if (section == MAP_REG_COLUMN)
                    return QStringLiteral("REG");
            }
        }
        default:
            return QVariant();
    }

    return QVariant();
}

Qt::ItemFlags mapModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return QAbstractItemModel::flags(index);

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;

    return flags;
}
