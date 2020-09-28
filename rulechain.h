#ifndef RULECHAIN_H
#define RULECHAIN_H

#include <QObject>
#include <QAbstractTableModel>

#define RULECHANCOLUMN 7

typedef struct rule_chain{
    QString ruleName;
    quint16 inDevId;
    quint16 inDevCh;
    quint16 ruleType;
    quint16 outDevId;
    quint16 outDevCh;
    quint16 seq;
} rule_chain;

class RuleChain: public QAbstractTableModel
{
public:
    RuleChain(QObject *parent = nullptr);
    void updateData(QList<rule_chain> recordList);
    void clearDate(QList<rule_chain>);
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
private:
    QList<rule_chain> m_recordList;
};
#endif // RULECHAIN_H
