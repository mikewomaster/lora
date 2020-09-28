#include <QModbusDataUnit>
#include <QModbusTcpClient>
#include <QModbusRtuSerialMaster>
#include <QByteArray>
#include <QProgressDialog>

#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "eventlogmodel.h"
#include "ymodem.h"

#define EVENTLOGENABLE (3001 - 1)
#define EVENTLOGHEAD (16)
#define EVENTLOGSIZE (20)

eventLogModel::eventLogModel(QObject *parent)
    : QAbstractTableModel(parent)
{

}

void eventLogModel::updateData(QList<eventLogData> recordList)
{
    m_recordList = recordList;

    beginResetModel();
    endResetModel();
}

void eventLogModel::clearDate(QList<eventLogData> recordList)
{
    updateData(recordList);
}

int eventLogModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_recordList.count();
}

int eventLogModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return EVENTLOGCOLUMN;
}

bool eventLogModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    Q_UNUSED(value);

    switch (role)
    {
        case Qt::EditRole:
        {
            break;
        }
        default:
            return false;
    }
    return false;
}

QVariant eventLogModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
           return QVariant();

       int nRow = index.row();
       int nColumn = index.column();
       eventLogData record = m_recordList.at(nRow);

       switch (role)
       {
           case Qt::TextAlignmentRole:
               return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
           case Qt::DisplayRole:
           {
               if (nColumn == 0)
                   return record.tag;

               else if (nColumn == 1)
                   return record.valueInput;

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
                   return record.valueOutput;

               else if (nColumn == 4) {
                   if (record.res == 1)
                       return "Success";
                   else
                       return "Fail";
               }

               else if (nColumn == 5) {
                   QDateTime time = QDateTime::fromTime_t(record.timestamp);
                   QString StrCurrentTime = time.toString("yyyy-MM-dd hh:mm:ss ddd");
                   return StrCurrentTime;
              }
               return QVariant();
           }
           default:
               return QVariant();
       }
       return QVariant();
}

QVariant eventLogModel::headerData(int section, Qt::Orientation orientation, int role) const
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
                    return QStringLiteral("Value Input");
                else if (section == 2)
                    return QStringLiteral("Type");
                else if (section == 3)
                    return QStringLiteral("Value Output");
                else if (section == 4)
                    return QStringLiteral("Result");
                else if (section == 5)
                    return QStringLiteral("TimeStamp");
            }
        }
        default:
            return QVariant();
    }
    return QVariant();
}

void eventLogModel::sort(int column, Qt::SortOrder order)
{
    if (column == 5) {
        std::sort(m_recordList.begin(), m_recordList.end(), compareEventTS);
    } else if (column == 0){
        std::sort(m_recordList.begin(), m_recordList.end(), compareEventTag);
    } else if (column == 2) {
        std::sort(m_recordList.begin(), m_recordList.end(), compareEventType);
    } else if (column == 4) {
        m_recordList = m_recordListBack;
    }

    updateData(m_recordList);
}

Qt::ItemFlags eventLogModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return QAbstractItemModel::flags(index);

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void MainWindow::on_eventLogEnableRadioButton_clicked()
{
    if (ui->eventLogEnableRadioButton->isChecked()) {
        ui->ELogChkPushButton->setEnabled(true);
    }
    else {
         ui->ELogChkPushButton->setEnabled(false);
    }

    if (!modbusDevice)
        return;

    if(modbusDevice->state() == QModbusDevice::UnconnectedState) {
        if (!m_ymodem->port)
            m_ymodem->port->close();
        modbusDevice->connectDevice();
        m_ymodem->dwnFlagRdy = false;
    }

    QModbusDataUnit writeUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, EVENTLOGENABLE, 1);

    if (ui->eventLogEnableRadioButton->isChecked())
        writeUnit.setValue(0, 1);
    else
        writeUnit.setValue(0, 0);

    handle_write(writeUnit);

}

void MainWindow::on_ELogClearPushButton_clicked()
{
    for (int i = 0; i < eventLogList.size(); i++) {
        eventLogList[i].tag = "";
        eventLogList[i].valueInput = 0;
        eventLogList[i].type = 0;
        eventLogList[i].valueOutput = 0;
        eventLogList[i].timestamp = 0;
        eventLogList[i].res = 0;
        ui->EventLogView->setRowHidden(i, true);
    }
}

void MainWindow::saveToTxt()
{
    QDateTime time = QDateTime::currentDateTime();
    QString currentTime = time.toString("yyyyMMddhhmmss");
    QString fileNameTxt = "EventLog" + currentTime + ".txt";

    QFile file;
    file.setFileName(fileNameTxt);
    file.open(QIODevice::Append);

    for (int i = 0; i < eventLogList.size(); i++) {
         if (eventLogList[i].tag == "")
            continue;
         else
        {
                const char *ch;
                QByteArray ba = eventLogList[i].tag.toLatin1();
                ch = ba.data();
                file.write(ch);

                char temp[5];
                memset(temp, 0, 5);
                sprintf(temp, ",%u,", eventLogList[i].valueInput);
                file.write(temp);

                if (eventLogList[i].type == 0) {
                    ch = "V-V";
                } else if (eventLogList[i].type == 1) {
                    ch = "V-A";
                } else if (eventLogList[i].type == 2) {
                    ch = "A-V";
                } else if (eventLogList[i].type == 3) {
                    ch = "A-A";
                }
                file.write(ch);

                memset(temp, 0, 5);
                sprintf(temp, ",%u,", eventLogList[i].valueOutput);
                file.write(temp);

                QDateTime time = QDateTime::fromTime_t(eventLogList[i].timestamp);
                QString StrCurrentTime = time.toString("yyyy-MM-dd hh:mm:ss");
                ba = StrCurrentTime.toLatin1();
                ch = ba.data();
                file.write(ch);
                ch = ",Fail\r\n";
                file.write(ch);
         }
    }
}

void MainWindow::parseEventLog(QString fileName)
{
    if(modbusDevice->state() == QModbusDevice::UnconnectedState) {
        modbusDevice->connectDevice();
        m_ymodem->port->close();
        m_ymodem->dwnFlagRdy = false;
    }

    modbusDevice->connectDevice();

    QFile *filep = new QFile(fileName);
    filep->open(QFile::ReadOnly);

    QByteArray dataAll;

    dataAll = filep->readAll();

    eventLogData record;

    eventLogList.clear();
    eLModel->m_recordListBack.clear();

    for (int i = 0; i < 1000; i++) {
        record.tag = "";
        record.valueInput = 0;
        record.type = 0;
        record.valueOutput = 0;
        record.timestamp = 0;
        record.res = 0;
        record.sequence = i;
        eventLogList.append(record);
        eLModel->m_recordListBack.append(record);
    }

    for (int i = 0; i < 1000; i++) {
        if (!dataAll[i*EVENTLOGSIZE + EVENTLOGHEAD])
            continue;

        for (int j = 0; j < 7; j++) {
            if (dataAll[j + i*EVENTLOGSIZE + EVENTLOGHEAD])
                eventLogList[i].tag[j] = dataAll[j + i*EVENTLOGSIZE + EVENTLOGHEAD];
        }
        eventLogList[i].valueInput = (dataAll[8 + i*EVENTLOGSIZE + EVENTLOGHEAD] & 0x00ff) + (dataAll[9 + i*EVENTLOGSIZE + EVENTLOGHEAD] << 8 & 0xff00);

        eventLogList[i].type = dataAll[10 + i*EVENTLOGSIZE + EVENTLOGHEAD] & 0x00ff;
        eventLogList[i].type |= (dataAll[11 + i*EVENTLOGSIZE + EVENTLOGHEAD] << 8)& 0xFF00;

        eventLogList[i].valueOutput = dataAll[12 + i*EVENTLOGSIZE + EVENTLOGHEAD] & 0x00FF;
        eventLogList[i].valueOutput |= (dataAll[13 + i*EVENTLOGSIZE + EVENTLOGHEAD] << 8) & 0xFF00;

        eventLogList[i].timestamp = dataAll[14 + i*EVENTLOGSIZE + EVENTLOGHEAD] & 0x000000FF;
        eventLogList[i].timestamp |= (dataAll[15 + i*EVENTLOGSIZE + EVENTLOGHEAD] << 8) & 0x0000FF00;
        eventLogList[i].timestamp |= (dataAll[16 + i*EVENTLOGSIZE + EVENTLOGHEAD] << 16) & 0x00FF0000;
        eventLogList[i].timestamp |= (dataAll[17 + i*EVENTLOGSIZE + EVENTLOGHEAD] << 24) & 0xFF000000;

        eventLogList[i].res = dataAll[18 + i*EVENTLOGSIZE + EVENTLOGHEAD];
        eventLogList[i].res |= dataAll[19 + i*EVENTLOGSIZE + EVENTLOGHEAD] & 0xFF00;
    }

    for (int i = 999; i >= 0; i--) {
          if (eventLogList[i].tag == "")
             eventLogList.removeAt(i);

         if (eventLogList[i].tag != "") {
            ui->EventLogView->setRowHidden(i, false);
         }
    }

    eLModel->updateData(eventLogList);
    eLModel->m_recordListBack = eventLogList;
    saveToTxt();

    ui->eventLogEnableRadioButton->setChecked(false);
    on_eventLogEnableRadioButton_clicked();

    filep->close();
    filep->remove();
    delete(filep);
}

void MainWindow::on_ELogChkPushButton_clicked()
{
    if (!ui->eventLogEnableRadioButton->isChecked())
        return;

    if (modbusDevice->state() == QModbusDevice::ConnectedState)
        modbusDevice->disconnectDevice();

    if (m_serial == nullptr) {
        m_serial = new QSerialPort();
        m_serial->setPortName(ui->portComboBox->currentText());
        m_serial->setBaudRate(QSerialPort::Baud9600);
        m_serial->setDataBits(QSerialPort::Data8);
        m_serial->setParity(QSerialPort::NoParity);
        m_serial->setStopBits(QSerialPort::OneStop);
        m_serial->setFlowControl(QSerialPort::NoFlowControl);
        m_serial->open(QIODevice::ReadWrite);
    } else {
        m_serial->setPortName(ui->portComboBox->currentText());
        m_serial->setBaudRate(QSerialPort::Baud9600);
        m_serial->setDataBits(QSerialPort::Data8);
        m_serial->setParity(QSerialPort::NoParity);
        m_serial->setStopBits(QSerialPort::OneStop);
        m_serial->setFlowControl(QSerialPort::NoFlowControl);
        m_serial->open(QIODevice::ReadWrite);
    }
    statusBar()->clearMessage();
    ui->ELogChkPushButton->setEnabled(false);

    m_ymodem->setPort(m_serial);
    if (!m_ymodem->pro)
        m_ymodem->pro = new QProgressDialog();
    m_ymodem->pro->setLabelText(tr("Processing... Please wait..."));
    m_ymodem->pro->setRange(0, 100);
    m_ymodem->pro->setModal(true);
    m_ymodem->pro->setCancelButtonText(tr("Cancel"));
    m_ymodem->pro->setValue(0);

    int i = 20;
    while(!m_ymodem->dwnFlagRdy && i--) {
            if (m_ymodem->port->isOpen())
                m_ymodem->startDownloadSingle();
            else
                break;
            _sleep(3000);
    }
}
