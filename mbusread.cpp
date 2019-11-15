#include "mbusread.h"
#include <QModbusDataUnit>
#include <QModbusTcpClient>
#include <QModbusRtuSerialMaster>
#include <QDebug>
#include <QTime>
#include <memory>

#include "ui_mainwindow.h"
#include "mainwindow.h"

#define DeviceNumber 20
#define DeviceHeadMemory 20
#define DeviceValueMemory 10

static  QList<QStandardItem *> item;

void MainWindow::on_clearViewBtn_clicked()
{
    m_Model->removeRows(0, m_Model->rowCount());

    // delete heap space
    if (storageItems.size()) {
        qDeleteAll(storageItems);
        storageItems.clear();
    }
}

void MainWindow::mbusReadDeviceReadReady()
{
    auto reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();
        QString s = "";
        unsigned char temp_2;
        unsigned int temp_4;
        item.clear();

        // HEAD Always 40 bytes, 20 regs
        // append device primary address
        temp_2 = unit.value(0) >> 8;
        QStandardItem *primaryAddr = new QStandardItem(QString::number(temp_2, 16).toUpper());
        item.append(primaryAddr);
        storageItems.push_back(primaryAddr);

        // append device primary mode
        temp_2 = (unit.value(0) & 0x00ff);
        QStandardItem *primaryMode = new QStandardItem(QString::number(temp_2, 16).toUpper());
        item.append(primaryMode);
        storageItems.push_back(primaryMode);

        // append ID Number
        temp_4 = unit.value(5);
        temp_4 = (temp_4 << 16) + unit.value(6);
        QStandardItem *id = new QStandardItem(QString::number(temp_4, 16).toUpper());
        item.append(id);
        storageItems.push_back(id);

        // append Man Number
        temp_2 = unit.value(7);
        QStandardItem *manu = new QStandardItem(QString::number(temp_2, 16).toUpper());
        item.append(new QStandardItem(QString::number(temp_2, 16).toUpper()));
        storageItems.push_back(manu);

        // append version
        temp_2 = unit.value(8) >> 8;
        QStandardItem *ver = new QStandardItem(QString::number(temp_2, 16).toUpper());
        item.append(ver);
        storageItems.push_back(ver);

        // append medium
        temp_2 = (unit.value(8) & 0x00ff);
        QStandardItem *medium = new QStandardItem(QString::number(temp_2, 16).toUpper());
        item.append(medium);
        storageItems.push_back(medium);

        // m_Model->appendRow(item);

        statusBar()->showMessage(tr("OK!"));
    } else if (reply->error() == QModbusDevice::ProtocolError) {
        statusBar()->showMessage(tr("Read response error: %1 (Mobus exception: 0x%2)").
                                    arg(reply->errorString()).
                                    arg(reply->rawResult().exceptionCode(), -1, 16), 5000);
    } else {
        statusBar()->showMessage(tr("Read response error: %1 (code: 0x%2)").
                                    arg(reply->errorString()).
                                    arg(reply->error(), -1, 16), 5000);
    }
    reply->deleteLater();
}

void MainWindow::mbusReadValueReadReady()
{
    auto reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();
        unsigned char temp_2;
        unsigned int temp_4;
        quint64 temp_8;

        for (int i = 0; i < 5; i++) {
            if (unit.value(i*10 + 0) == 0xffff )
                continue;

            // append type
            temp_2 = unit.value( i*10 + 0) >> 8;
            QStandardItem *type = new QStandardItem(QString::number(temp_2, 16).toUpper());
            storageItems.push_back(type);
            item.append(type);

            // append unit
            temp_2 = (unit.value(i*10+0) & 0x00ff);
            QStandardItem* uni = new QStandardItem(QString::number(temp_2, 16).toUpper());
            storageItems.push_back(uni);
            item.append(uni);

            // append scale
            temp_4 = unit.value(i*10+1);
            QStandardItem *scale = new QStandardItem(QString::number(temp_4, 16).toUpper());
            storageItems.push_back(scale);
            item.append(scale);

            // append value
            memset(&temp_8, 0, sizeof(temp_8));
            for (int j = 0; j < 4; j++) {
               temp_8 = temp_8 << 16;
               temp_8 = temp_8 + unit.value(i*10 + 2 + j);
            }
            QStandardItem *va = new QStandardItem(QString::number(temp_8, 16).toUpper());
            storageItems.push_back(va);
            item.append(va);

            // append timestamp
            quint64 data;
            data = unit.value(i*10 + 8);
            data = (data << 16) + unit.value(i*10 + 9);
            QStandardItem* ts = new QStandardItem(QString::number(data, 16).toUpper());
            storageItems.push_back(ts);
            item.append(ts);

            m_Model->appendRow(item);
            for (int i = 0; i < 5 ; i++){ // remove no.6/7/8/9/10
                item.removeAt(6);
            }
        }
        statusBar()->showMessage(tr("OK!"));
    } else if (reply->error() == QModbusDevice::ProtocolError) {
        statusBar()->showMessage(tr("Read response error: %1 (Mobus exception: 0x%2)").
                                    arg(reply->errorString()).
                                    arg(reply->rawResult().exceptionCode(), -1, 16), 5000);
    } else {
        statusBar()->showMessage(tr("Read response error: %1 (code: 0x%2)").
                                    arg(reply->errorString()).
                                    arg(reply->error(), -1, 16), 5000);
    }
    reply->deleteLater();
}

void MainWindow::on_mbusPrimaryRead_clicked()
{
    if (!modbusDevice)
        return;
    statusBar()->clearMessage();

    QString text = ui->mbusPrimaryEdit->text();
    QStringList textList = text.split(",");

    for (int i = 0; i < textList.size(); i++) {
        int multiplyTerm = textList.at(i).toInt() - 1;
        quint16 ADDR = mbusReadDeviceRAMAddr + mbusReadDeviceRAMEntries * multiplyTerm;

        QModbusDataUnit readUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, ADDR, mbusReadDeviceHead);
        if (auto *reply = modbusDevice->sendReadRequest(readUnit, ui->serverEdit->value())) {
            if (!reply->isFinished())
                connect(reply, &QModbusReply::finished, this, &MainWindow::mbusReadDeviceReadReady);
            else
                delete reply; // broadcast replies return immediately
        } else {
            statusBar()->showMessage(tr("Read error: ") + modbusDevice->errorString(), 5000);
        }

        // sleep 1 second for signal delay caused by transparation by serial
        _sleep(1000);

        // LOOP 4 times to read other values which contains 50 units(100 bytes)
        for (int j = 0; j < 4; j++) {
            quint16 ADDR = mbusReadDeviceRAMAddr + mbusReadDeviceRAMEntries * multiplyTerm + mbusReadDeviceHead + j*mbusReadDeviceValue;
            QModbusDataUnit readUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, ADDR, mbusReadDeviceValue);

            if (auto *reply = modbusDevice->sendReadRequest(readUnit, ui->serverEdit->value())) {
                if (!reply->isFinished()) {
                    connect(reply, &QModbusReply::finished, this, &MainWindow::mbusReadValueReadReady);
                }
                else
                    delete reply; // broadcast replies return immediately
            } else {
                statusBar()->showMessage(tr("Read error: ") + modbusDevice->errorString(), 5000);
            }

            // delay 1 second
            _sleep(5000);
        }
    }
}
