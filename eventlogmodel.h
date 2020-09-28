#ifndef EVENTLOGMODEL_H
#define EVENTLOGMODEL_H

#include <QObject>
#include <QAbstractTableModel>
#include <QAbstractProxyModel>
#include <QStandardItemModel>

#define EVENTLOGCOLUMN 6

typedef struct eventLogData{
    QString tag;
    quint16 valueInput;
    quint16 type;
    quint16 valueOutput;
    quint32 timestamp;
    quint16 res;
    quint16 sequence;
} eventLogData;

class eventLogModel: public QAbstractTableModel
{
public:
    eventLogModel(QObject *parent = nullptr);
    void updateData(QList<eventLogData>);
    void clearDate(QList<eventLogData>);
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    void sort(int column, Qt::SortOrder order) override;
    static bool compareEventTS(const eventLogData& l1, const eventLogData& l2){
         return (l1.timestamp < l2.timestamp);
    }
    static bool compareEventTag(const eventLogData& l1, const eventLogData& l2){
         return (l1.tag < l2.tag);
    }
    static bool compareEventType(const eventLogData& l1, const eventLogData& l2){
         return (l1.type < l2.type);
    }
    QList<eventLogData> m_recordListBack;

private:
    QList<eventLogData> m_recordList;
};

#endif // EVENTLOGMODEL_H
