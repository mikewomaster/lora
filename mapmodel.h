#ifndef MAPMODEL_H
#define MAPMODEL_H

#include <QObject>
#include <QAbstractTableModel>

typedef struct mapModelData{
    quint16 id;
    quint16 reg;
}mapModelData;

class mapModel: public QAbstractTableModel
{
public:
    mapModel(QObject *parent = nullptr);
    void updateData(QList<mapModelData> recordList);
    void clearDate();
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
private:
    QList<mapModelData> m_recordList;
};

#endif // MAPMODEL_H
