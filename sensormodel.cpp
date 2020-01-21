#include "sensor.h"
#include "mainwindow.h"
#include <QAbstractTableModel>
#include <QMessageBox>
#include <QModbusDataUnit>
#include <QModbusTcpClient>
#include <QModbusRtuSerialMaster>

static int times = 1;

void MainWindow::sensorAddModbus(sen unit, int term)
{
    if (!modbusDevice)
        return;

    QModbusDataUnit writeUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, RTUSENSORADDR + term*RTUSENSORNUM, RTUSENSORNUM);

    writeUnit.setValue(0, unit.type_);
    writeUnit.setValue(1, unit.id);
    writeUnit.setValue(2, (unit.reg_addr - 40001));

    if (auto *reply = modbusDevice->sendWriteRequest(writeUnit, ui->serverEdit->value())) {
        if (!reply->isFinished()) {
            connect(reply, &QModbusReply::finished, this, [this, reply]() {
                if (reply->error() == QModbusDevice::ProtocolError) {
                    statusBar()->showMessage(tr("Write response error: %1 (Mobus exception: 0x%2)")
                        .arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16),
                        5000);
                } else if (reply->error() != QModbusDevice::NoError) {
                    statusBar()->showMessage(tr("Write response error: %1 (code: 0x%2)").
                        arg(reply->errorString()).arg(reply->error(), -1, 16), 5000);
                }
                statusBar()->showMessage(tr("OK!"));
                reply->deleteLater();
            });
        } else {
            // broadcast replies return immediately
            reply->deleteLater();
        }
    } else {
        statusBar()->showMessage(tr("Write error: ") + modbusDevice->errorString(), 5000);
    }
}

void MainWindow::on_sensorAddPushButton_clicked()
{
    statusBar()->clearMessage();

    sen unit;
    unit.type = ui->sensorTypeCombox->currentText();
    unit.type_ = ui->sensorTypeCombox->currentIndex() + 1;
    unit.id = ui->sensorSlaveId->text().toShort();
    unit.reg_addr = ui->sensorPLCAddress->text().toInt();
    unit.value = "null";

    int i = 0;
    for (i = 0; i < 50; i++){
        if (sensorRecordList[i].type == ""){
            int term = i;
            sensorAddModbus(unit, term);
            unit.seq = i;
            sensorRecordList[i] = unit;
            break;
        }
    }

    m_sensorModel->updateData(sensorRecordList);
    ui->sensorTableView->setRowHidden(i, false);
}

void MainWindow::on_sensorClearPushButton_clicked()
{
    for (int i =0; i < 50; i++) {
        sensorRecordList[i].type = "";
        sensorRecordList[i].type_ = 0;
        sensorRecordList[i].id = 0;
        sensorRecordList[i].reg_addr = 0;
        ui->sensorTableView->setRowHidden(i, true);
    }
}

// MAGIC NUMBER: 2 times, each 25 structs, 3 values (number, id, reg_addr), total 75 bytes
void MainWindow::sensorUpdateReadReady()
{
    auto reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

    sen s;
    if (reply->error() == QModbusDevice::NoError) {
        m_sensorModel->clearDate(sensorRecordList);
        const QModbusDataUnit unit = reply->result();
        for (int i = 0; i < 25; i++) {
            if (times == 1) {
                sensorRecordList[i].type_ = unit.value(3*i + 0);
                sensorRecordList[i].type = sl.at(s.type_);
                sensorRecordList[i].id = unit.value(3*i + 1);
                sensorRecordList[i].reg_addr = unit.value(3*i +2);
            } else {
                sensorRecordList[25+i].type_ = unit.value(3*i + 0);
                sensorRecordList[25+i].type = sl.at(s.type_);
                sensorRecordList[25+i].id = unit.value(3*i + 1);
                sensorRecordList[25+i].reg_addr = unit.value(3*i +2);
            }
        }
        times == 1 ? 2 : 1;
        sensorFlag = true;
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

void MainWindow::on_sensorUpdatePushButton_clicked()
{
    if (!modbusDevice)
        return;
    statusBar()->clearMessage();

    // MAGIC NUMBER: 2 times, each 25 structs, with 3 bytes, total 75 bytes
    for (int j = 0; j < 2; j++) {
        quint16 ADDR = RTUSENSORADDR + 75 * j;
        QModbusDataUnit readUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, ADDR, 75);

        if (auto *reply = modbusDevice->sendReadRequest(readUnit, ui->serverEdit->value())) {
            if (!reply->isFinished()) {
                connect(reply, &QModbusReply::finished, this, &MainWindow::sensorUpdateReadReady);
            }
            else
                delete reply;
        } else {
            statusBar()->showMessage(tr("Read error: ") + modbusDevice->errorString(), 5000);
        }
        // delay 3 seconds
        _sleep(3000);
    }

    m_sensorModel->updateData(sensorRecordList);
    for (int i = 0; i < 50; i++) {
         if (sensorRecordList[i].type != "")
            ui->sensorTableView->setRowHidden(i, false);
    }
}

void MainWindow::sensorCheckReadReady()
{
    auto reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

    sen s;
    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();
        for (int i = 0; i < 50; i++) {
            sensorRecordList[i].value = unit.value(i);
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

void MainWindow::on_sensorCheckPushButton_clicked()
{
    if (!modbusDevice)
        return;
    statusBar()->clearMessage();

    if (!sensorFlag) {
        QMessageBox::information(NULL, "Information", "Please Update Sensor Configure Firstly");
        return;
    }

    quint16 ADDR = RTUSENSORVALUE;
    QModbusDataUnit readUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, ADDR, RTUVALUEENTRIES);

    if (auto *reply = modbusDevice->sendReadRequest(readUnit, ui->serverEdit->value())) {
        if (!reply->isFinished()) {
            connect(reply, &QModbusReply::finished, this, &MainWindow::sensorCheckReadReady);
        }
        else
            delete reply;
    } else {
        statusBar()->showMessage(tr("Read error: ") + modbusDevice->errorString(), 5000);
    }

    for (int i = 0; i < 50; i++) {
         if (sensorRecordList[i].type != "")
            ui->sensorTableView->setRowHidden(i, false);
    }
}

void MainWindow::sensMensu(QPoint pos)
{
    QModelIndex index = ui->sensorTableView->indexAt(pos);
    if (index.isValid()) {
        senpopMenu->exec(QCursor::pos());
    }
}

void MainWindow::sensEdit()
{
    int row = ui->sensorTableView->currentIndex().row();
    QAbstractItemModel *model = ui->sensorTableView->model();
    QModelIndex index = model->index(row, 4);
    QVariant data = model->data(index);
    int term = data.toInt();
    if (!modbusDevice)
        return;

    QModbusDataUnit writeUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, RTUSENSORADDR + term*RTUSENSORNUM, RTUSENSORNUM);
    m_sensor_dialog->seq = term;
    m_sensor_dialog->show();
    while(!sensor_edit_flag){
        _sleep(500);
    }
    // FIX ME
    writeUnit.setValue(0, sensorRecordList[term].type_);
    writeUnit.setValue(1, sensorRecordList[term].id);
    writeUnit.setValue(2, sensorRecordList[term].reg_addr);

    if (auto *reply = modbusDevice->sendWriteRequest(writeUnit, ui->serverEdit->value())) {
        if (!reply->isFinished()) {
            connect(reply, &QModbusReply::finished, this, [this, reply]() {
                if (reply->error() == QModbusDevice::ProtocolError) {
                    statusBar()->showMessage(tr("Write response error: %1 (Mobus exception: 0x%2)")
                        .arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16),
                        5000);
                } else if (reply->error() != QModbusDevice::NoError) {
                    statusBar()->showMessage(tr("Write response error: %1 (code: 0x%2)").
                        arg(reply->errorString()).arg(reply->error(), -1, 16), 5000);
                }
                statusBar()->showMessage(tr("OK!"));
                reply->deleteLater();
            });
        } else {
            // broadcast replies return immediately
            reply->deleteLater();
        }
    } else {
        statusBar()->showMessage(tr("Write error: ") + modbusDevice->errorString(), 5000);
    }
    m_sensorModel->updateData(sensorRecordList);
    sensor_edit_flag = false;
}

void MainWindow::sensDelete()
{
    int row = ui->sensorTableView->currentIndex().row();
    QAbstractItemModel *model = ui->sensorTableView->model();
    QModelIndex index = model->index(row, 4);
    QVariant data = model->data(index);
    int term = data.toInt();

    sensorRecordList[term].type = "";
    sensorRecordList[term].type_ = 0;
    sensorRecordList[term].id = 0;
    sensorRecordList[term].reg_addr = 0;

    if (!modbusDevice)
        return;

    QModbusDataUnit writeUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, RTUSENSORADDR + term*RTUSENSORNUM, RTUSENSORNUM);

    writeUnit.setValue(0, sensorRecordList[term].type_);
    writeUnit.setValue(1, sensorRecordList[term].id);
    writeUnit.setValue(2, sensorRecordList[term].reg_addr);

    if (auto *reply = modbusDevice->sendWriteRequest(writeUnit, ui->serverEdit->value())) {
        if (!reply->isFinished()) {
            connect(reply, &QModbusReply::finished, this, [this, reply]() {
                if (reply->error() == QModbusDevice::ProtocolError) {
                    statusBar()->showMessage(tr("Write response error: %1 (Mobus exception: 0x%2)")
                        .arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16),
                        5000);
                } else if (reply->error() != QModbusDevice::NoError) {
                    statusBar()->showMessage(tr("Write response error: %1 (code: 0x%2)").
                        arg(reply->errorString()).arg(reply->error(), -1, 16), 5000);
                }
                statusBar()->showMessage(tr("OK!"));
                reply->deleteLater();
            });
        } else {
            // broadcast replies return immediately
            reply->deleteLater();
        }
    } else {
        statusBar()->showMessage(tr("Write error: ") + modbusDevice->errorString(), 5000);
    }
    ui->sensorTableView->setRowHidden(term, true);
}

void MainWindow::sensor_view_model()
{
    m_sensorModel = new sensor(this);
    ui->sensorTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->sensorTableView->setShowGrid(true);
    ui->sensorTableView->setFrameShape(QFrame::Box);
    ui->sensorTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->sensorTableView->setModel(m_sensorModel);
    ui->sensorTableView->setColumnWidth(0, 150);
    ui->sensorTableView->setColumnWidth(1, 140);
    ui->sensorTableView->setColumnWidth(2, 120);
    ui->sensorTableView->setColumnWidth(3, 110);
    ui->sensorTableView->setColumnWidth(4, 70);
    ui->sensorTableView->setContextMenuPolicy(Qt::CustomContextMenu);

    senpopMenu = new QMenu(ui->obisView);
    QAction *actionUpdateSensInfo = new QAction();
    QAction *actionDelSensInfo = new QAction();
    actionUpdateSensInfo ->setText(QString("Edit"));
    actionDelSensInfo ->setText(QString("Delete"));
    senpopMenu->addAction(actionUpdateSensInfo);
    senpopMenu->addAction(actionDelSensInfo);
    connect(ui->sensorTableView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(sensMensu(QPoint)));
    connect(actionUpdateSensInfo, &QAction::triggered, this, &MainWindow::sensEdit);
    connect(actionDelSensInfo, &QAction::triggered, this, &MainWindow::sensDelete);

    sensorRecordList.clear();
    sen record;
    for (int i = 0; i < 50; i++) {
        record.id = 0;
        record.reg_addr = 0;
        record.type_ = 0;
        record.seq = i;
        sensorRecordList.append(record);
    }
    m_sensorModel->updateData(sensorRecordList);
    for (int i = 0; i < 50; i++) {
        if (!record.type_)
            ui->sensorTableView->setRowHidden(i, true);
    }
    ui->sensorTableView->setColumnHidden(4, true);
}
