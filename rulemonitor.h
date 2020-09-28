#ifndef RULEMONITOR_H
#define RULEMONITOR_H
#include <QObject>
#include <QAbstractTableModel>

#define RULEMonitorCOLUMN 5

#define ruleMonitorAddress (2001 - 1)
#define ruleMonitorUnitLength 9
#define ruleMonitorTotalTable 40
#define ruleMonitorTableTimes 4
#define ruleMonitorTableEachTimeUnit 90
#define ruleMonitorUnitEachTime 10

typedef struct ruleMonitorStruct{
    QString tag;
    quint16 value;
    quint16 type;
    quint16 result;
    quint32 timestamp;
    QString ts;
    quint16 sequence;
} ruleMonitorStruct;

class ruleMonitor: public QAbstractTableModel
{
public:
    ruleMonitor(QObject *parent = nullptr);
    void updateData(QList<ruleMonitorStruct>);
    void clearDate(QList<ruleMonitorStruct>);
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
private:
    QList<ruleMonitorStruct> m_recordList;
};

#endif // RULEMONITOR_H
