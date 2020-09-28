#include <QMessageBox>
#include <QModbusDataUnit>
#include <QModbusTcpClient>
#include <QAbstractTableModel>
#include <QModbusRtuSerialMaster>

#include "rulemonitor.h"
#include "mainwindow.h"

/*
 * rule Monitor Model-View Page
*/
ruleMonitor::ruleMonitor(QObject *parent)
    : QAbstractTableModel(parent)
{

}

void ruleMonitor::updateData(QList<ruleMonitorStruct> recordList)
{
    m_recordList = recordList;
    beginResetModel();
    endResetModel();
}

void ruleMonitor::clearDate(QList<ruleMonitorStruct> recordList)
{
    updateData(recordList);
}

int ruleMonitor::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_recordList.count();
}

int ruleMonitor::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return RULEMonitorCOLUMN;
}

bool ruleMonitor::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    int nColumn = index.column();
    ruleMonitorStruct record = m_recordList.at(index.row());

    switch (role)
    {
        case Qt::EditRole:
        {
            if (nColumn == 0)
            {
                record.tag = value.toString();
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

QVariant ruleMonitor::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int nRow = index.row();
    int nColumn = index.column();
    ruleMonitorStruct record = m_recordList.at(nRow);

    switch (role)
    {
        case Qt::TextAlignmentRole:
            return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
        case Qt::DisplayRole:
        {
            if (nColumn == 0)
                return record.tag;

            else if (nColumn == 1)
                return record.value;

            else if (nColumn == 2) {
                if (record.type == 0)
                    return "V-V";
                else if (record.type == 1)
                    return "V-A";
                else if (record.type == 2)
                    return "A-V";
                else if (record.type == 3)
                    return "A-A";
            }

            else if (nColumn == 3)
                if (record.result == 1)
                    return "Success";
                else
                    return "Fail";

            else if (nColumn == 4)
                return record.ts;

            return QVariant();
        }
        default:
            return QVariant();
    }
    return QVariant();
}

QVariant ruleMonitor::headerData(int section, Qt::Orientation orientation, int role) const
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
                    return QStringLiteral("Value");
                else if (section == 2)
                    return QStringLiteral("Type");
                else if (section == 3)
                    return QStringLiteral("Result");
                else if (section == 4)
                    return QStringLiteral("TimeStamp");
            }
        }
        default:
            return QVariant();
    }
    return QVariant();
}

Qt::ItemFlags ruleMonitor::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return QAbstractItemModel::flags(index);

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

/*
 *rule monitor page - Modbus
 * #define ruleMonitorAddress (2001 - 1)
    #define ruleMonitorUnitLength 9
    #define ruleMonitorTotalTable 40
    #define ruleMonitorTableTimes 4
    #define ruleMonitorTableEachTimeUnit 90
    #define ruleMonitorUnitEachTime 10
*/
static int __ruleMontorTime;
void MainWindow::ruleMonitorTableReadReady()
{
    auto reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

    if (reply->error() == QModbusDevice::NoError) {
        m_RuleMonitorModel->clearDate(ruleMonitorList);
        const QModbusDataUnit unit = reply->result();

        for (int i = 0; i < ruleMonitorUnitEachTime; i++) {
            QString s = "";
            int stringNumber = 0;
            for (int j = ruleMonitorUnitLength*i + 0; j < ruleMonitorUnitLength*i + 4; j++) {
                if ((unit.value(j) >> 8) == 0x00)
                    break;
                s[2*stringNumber] = unit.value(j) >> 8;
                if ((unit.value(j) & 0x00ff) == 0x00)
                    break;
                s[(2*stringNumber) +1] = unit.value(j) & 0x00ff;
                stringNumber ++;
            }
            ruleMonitorList[(ruleMonitorUnitEachTime*__ruleMontorTime)+i].tag = s;
            ruleMonitorList[(ruleMonitorUnitEachTime*__ruleMontorTime)+i].value = unit.value(ruleMonitorUnitLength*i + 4);
            ruleMonitorList[(ruleMonitorUnitEachTime*__ruleMontorTime)+i].type = unit.value(ruleMonitorUnitLength*i + 5);
            ruleMonitorList[(ruleMonitorUnitEachTime*__ruleMontorTime)+i].result = unit.value(ruleMonitorUnitLength*i + 6);

            quint32 ts_ = unit.value(ruleMonitorUnitLength*i + 7);
            ts_ = (ts_ << 16) + unit.value(ruleMonitorUnitLength*i + 8);
            QDateTime time = QDateTime::fromTime_t(ts_);
            QString StrCurrentTime = time.toString("yyyy-MM-dd hh:mm:ss ddd");
            ruleMonitorList[(ruleMonitorUnitEachTime*__ruleMontorTime)+i].ts = StrCurrentTime;
        }
    }else if (reply->error() == QModbusDevice::ProtocolError) {
            statusBar()->showMessage(tr("Read response error: %1 (Mobus exception: 0x%2)").
                                        arg(reply->errorString()).
                                        arg(reply->rawResult().exceptionCode(), -1, 16), 5000);
    } else {
        statusBar()->showMessage(tr("Read response error: %1 (code: 0x%2)").
                                    arg(reply->errorString()).
                                    arg(reply->error(), -1, 16), 5000);
    }
    (__ruleMontorTime == 3) ? (__ruleMontorTime = 0) : (__ruleMontorTime++);
    reply->deleteLater();
}

void MainWindow::on_monitorCheckpushButton_clicked()
{
    if (!modbusDevice)
        return;
    statusBar()->clearMessage();
    ui->monitorCheckpushButton->setEnabled(false);
    for (int i = 0; i < ruleMonitorTableTimes; i++) {
        quint16 ADDR =  ruleMonitorAddress + (ruleMonitorTableEachTimeUnit * i);
        QModbusDataUnit readUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, ADDR, ruleMonitorTableEachTimeUnit);

        if (auto *reply = modbusDevice->sendReadRequest(readUnit, ui->serverEdit->value())) {
            if (!reply->isFinished()) {
                connect(reply, &QModbusReply::finished, this, &MainWindow::ruleMonitorTableReadReady);
            }
            else
                delete reply;
        } else {
            statusBar()->showMessage(tr("Read error: ") + modbusDevice->errorString(), 5000);
        }
        _sleep(1000);
    }

    _sleep(6000);

    // QMessageBox::information(nullptr, "Read Value", "Read rule monitor result done, thanks for patience.");
    m_RuleMonitorModel->updateData(ruleMonitorList);

    for (int i = 0; i < ruleMonitorTotalTable; i++) {
         if (ruleMonitorList[i].tag != "")
            ui->MonitorView->setRowHidden(i, false);
    }
    ui->monitorCheckpushButton->setEnabled(true);
}

void MainWindow::on_monitorClearPushButton_clicked()
{
    for (int i = 0; i < 40; i++) {
        ruleMonitorList[i].tag = "";
        ruleMonitorList[i].value = 0;
        ruleMonitorList[i].type = 0;
        ruleMonitorList[i].result = 0;
        ruleMonitorList[i].timestamp = 0;
        ruleMonitorList[i].ts = "";
        ui->MonitorView->setRowHidden(i, true);
    }
}

void MainWindow::on_monitorRadioButton_clicked()
{
    if (monitorAlarm == nullptr) {
        monitorAlarm = new QTimer();
        monitorAlarm->setInterval(60000);
        connect(monitorAlarm, SIGNAL(timeout()), this, SLOT(on_monitorCheckpushButton_clicked()));
    }

    if (ui->monitorRadioButton->isChecked()) {
        monitorAlarm->start();
    } else {
        monitorAlarm->stop();
    }
}
