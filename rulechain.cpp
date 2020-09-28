#include <QMessageBox>
#include <QModbusDataUnit>
#include <QModbusTcpClient>
#include <QAbstractTableModel>
#include <QModbusRtuSerialMaster>

#include "rulechain.h"
#include "mainwindow.h"

RuleChain::RuleChain(QObject *parent)
    : QAbstractTableModel(parent)
{

}

void RuleChain::updateData(QList<rule_chain> recordList)
{
    m_recordList = recordList;
    beginResetModel();
    endResetModel();
}

void RuleChain::clearDate(QList<rule_chain> recordList)
{
    updateData(recordList);
}

int RuleChain::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_recordList.count();
}

int RuleChain::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return RULECHANCOLUMN;
}

bool RuleChain::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    int nColumn = index.column();
    rule_chain record = m_recordList.at(index.row());

    switch (role)
    {
        case Qt::EditRole:
        {
            if (nColumn == 0)
            {
                record.ruleName = value.toString();
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

QVariant RuleChain::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int nRow = index.row();
    int nColumn = index.column();
    rule_chain record = m_recordList.at(nRow);

    switch (role)
    {
        case Qt::TextAlignmentRole:
            return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
        case Qt::DisplayRole:
        {
            if (nColumn == 0)
                return record.ruleName;
            else if (nColumn == 1)
                return record.inDevId;
            else if (nColumn == 2)
                return record.inDevCh;
            else if (nColumn == 3) {
                if (record.ruleType == 0)
                    return "V-V";
                else if (record.ruleType == 1)
                    return "V-A";
                else if (record.ruleType == 2)
                    return "A-V";
                else if (record.ruleType == 3)
                    return "A-A";
            }
            else if (nColumn == 4)
                return record.outDevId;
            else if (nColumn == 5)
                return record.outDevCh;
            else if (nColumn == 6)
                return record.seq;

            return QVariant();
        }
        default:
            return QVariant();
    }
    return QVariant();
}

QVariant RuleChain::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (role)
    {
        case Qt::TextAlignmentRole:
            return QVariant(Qt::AlignCenter | Qt::AlignVCenter);

        case Qt::DisplayRole:
        {
            if (orientation == Qt::Horizontal)
            {
                if (section == 0)
                    return QStringLiteral("Tag");
                else if (section == 1)
                    return QStringLiteral("IN Dev_ID");
                else if (section == 2)
                    return QStringLiteral("IN_CH");
                else if (section == 3)
                    return QStringLiteral("Type");
                else if (section == 4)
                    return QStringLiteral("OUT Dev_ID");
                else if (section == 5)
                    return QStringLiteral("OUT_CH");
            }
        }
        default:
            return QVariant();
    }
    return QVariant();
}

Qt::ItemFlags RuleChain::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return QAbstractItemModel::flags(index);

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void MainWindow::ruleChainMenuPaint(QPoint pos)
{
    QModelIndex index = ui->RuleVIew ->indexAt(pos);
    if (index.isValid()) {
        ruleChainMenu->exec(QCursor::pos());
    }
}
