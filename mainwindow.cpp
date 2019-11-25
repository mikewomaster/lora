/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the QtSerialBus module.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsdialog.h"
#include "logdialog.h"
#include "system.h"
#include "writeregistermodel.h"

#include <QModbusTcpClient>
#include <QModbusRtuSerialMaster>
#include <QStandardItemModel>
#include <QStatusBar>
#include <QUrl>
#include <QtSerialPort/QSerialPortInfo>
#include <QDebug>
#include <QTime>
#include <QSize>

enum ModbusConnection {
    Serial,
    Tcp
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , lastRequest(nullptr)
    , modbusDevice(nullptr)
    , m_Model(new QStandardItemModel())
    , m_serial(new QSerialPort)
{
    ui->setupUi(this);

    m_settingsDialog = new SettingsDialog(this);
    m_logdialog = new logdialog(this);
    m_system = new systemDialog(this);
    initActions();

    writeModel = new WriteRegisterModel(this);

    ui->connectType->setCurrentIndex(0);
    on_connectType_currentIndexChanged(0);

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
       ui->portComboBox->addItem(info.portName());

    m_Model->setHorizontalHeaderItem(0, new QStandardItem(QObject::tr("Address")));
    m_Model->setHorizontalHeaderItem(1, new QStandardItem(QObject::tr("Mode")));
    m_Model->setHorizontalHeaderItem(2, new QStandardItem(QObject::tr("SerialNumber")));
    m_Model->setHorizontalHeaderItem(3, new QStandardItem(QObject::tr("Manu")));
    m_Model->setHorizontalHeaderItem(4, new QStandardItem(QObject::tr("Version")));
    m_Model->setHorizontalHeaderItem(5, new QStandardItem(QObject::tr("Medium")));
    // m_Model->setHorizontalHeaderItem(6, new QStandardItem(QObject::tr("Type")));
    m_Model->setHorizontalHeaderItem(6, new QStandardItem(QObject::tr("Value")));
    m_Model->setHorizontalHeaderItem(7, new QStandardItem(QObject::tr("Unit")));
    // m_Model->setHorizontalHeaderItem(8, new QStandardItem(QObject::tr("Scale")));
    m_Model->setHorizontalHeaderItem(8, new QStandardItem(QObject::tr("Description")));
    m_Model->setHorizontalHeaderItem(9, new QStandardItem(QObject::tr("TimeStamp")));

    ui->connectType->hide();
    ui->serverEdit->hide();
    ui->syncWordLineEdit->hide();
    ui->OptimizeLowRateComboBox->hide();
    ui->tabWidget_2->show();
    ui->tabWidget_2->setTabEnabled(5, false);
    ui->tabWidget_2->setStyleSheet("QTabBar::tab:disabled {width: 0; color: transparent;}");
    ui->tabWidget->hide();
    ui->groupBox_6->hide();
    ui->groupBox_11->hide();
    ui->groupBox_13->hide();
    ui->mbusRegister->hide();
    ui->mbusPrimaryEdit->setText("1");
    ui->mbusPrimaryEdit->hide();

    // hide button
    ui->mbusPrimaryRead_12->hide();
    ui->mbusPrimaryWrite_11->hide();
    ui->mbusSecondaryRead->hide();
    ui->mbusSecondaryWrite->hide();
    ui->mbusReadoutRead->hide();
    ui->mbusReadoutWrite->hide();
    ui->mbusTSRead->hide();
    ui->mbusTSWrite->hide();

    ui->apnRead->hide();
    ui->apnWrite->hide();
    ui->userRead->hide();
    ui->userWrite->hide();
    ui->passwordRead->hide();
    ui->passwordWrite->hide();
    ui->nbStatusRead->hide();
    ui->nbStatusWrite->hide();
    ui->ipRead->hide();
    ui->ipWrite->hide();

    ui->srvRead->hide();
    ui->srvWrite->hide();
    ui->portRead->hide();
    ui->portWrite->hide();
    ui->topicRead->hide();
    ui->topicWrite->hide();
    ui->idRead->hide();
    ui->idWrite->hide();
    ui->intervalRead->hide();
    ui->idWrite_2->hide();
    ui->mqttStatusRead->hide();
    ui->pidButtonWrite_8->hide();

#ifdef TEST_DATA
    QList<QStandardItem *> item;
    item.append(new QStandardItem(QObject::tr("Always")));
    item.append(new QStandardItem(QObject::tr("primary")));
    item.append(new QStandardItem(QObject::tr("201911040001")));
    item.append(new QStandardItem(QObject::tr("WM")));
    item.append(new QStandardItem(QObject::tr("0.0.1")));
    item.append(new QStandardItem(QObject::tr("Heat")));
    item.append(new QStandardItem(QObject::tr("2 BCD")));
    item.append(new QStandardItem(QObject::tr("J/h")));
    item.append(new QStandardItem(QObject::tr("0.001")));
    item.append(new QStandardItem(QObject::tr("99.6")));
    item.append(new QStandardItem(QObject::tr("HMS Heat")));
    m_Model->appendRow(item);
    item.clear();

    item.append(new QStandardItem(QObject::tr("2")));
    item.append(new QStandardItem(QObject::tr("primary")));
    item.append(new QStandardItem(QObject::tr("201911040001")));
    item.append(new QStandardItem(QObject::tr("WM")));
    item.append(new QStandardItem(QObject::tr("0.0.1")));
    item.append(new QStandardItem(QObject::tr("Heat")));
    item.append(new QStandardItem(QObject::tr("2 BCD")));
    item.append(new QStandardItem(QObject::tr("J/h")));
    item.append(new QStandardItem(QObject::tr("0.001")));
    item.append(new QStandardItem(QObject::tr("99.6")));
    item.append(new QStandardItem(QObject::tr("HMS Heat")));
    m_Model->appendRow(item);
    item.clear();
#endif

    ui->tableView->setModel(m_Model);

    // resize
#if 0
    QWidget *widgetAdjust = new QWidget(this);
    setCentralWidget(widgetAdjust);
    QVBoxLayout *windowLayoutAdjust = new QVBoxLayout;
    windowLayoutAdjust->addLayout(ui->gridLayout, 1);
    windowLayoutAdjust->addWidget(ui->tabWidget_2);
    widgetAdjust->setLayout(windowLayoutAdjust);
#endif
    //
}

MainWindow::~MainWindow()
{
    if (modbusDevice)
        modbusDevice->disconnectDevice();
    delete modbusDevice;

    delete ui;
}

void MainWindow::initActions()
{
    ui->actionConnect->setEnabled(true); 
    ui->actionDisconnect->setEnabled(false);
    ui->actionExit->setEnabled(true);
    ui->actionOptions->setEnabled(true);

    connect(ui->actionConnect, &QAction::triggered,this, &MainWindow::on_connectButton_clicked);
    connect(ui->actionDisconnect, &QAction::triggered,this, &MainWindow::on_connectButton_clicked);
    connect(ui->actionExit, &QAction::triggered, this, &QMainWindow::close);
    connect(ui->actionOptions, &QAction::triggered, m_settingsDialog, &QDialog::show);
    connect(ui->actionLog, &QAction::triggered, m_logdialog, &QDialog::show);

    ui->action_Settings->setEnabled(true);
    connect(ui->action_Settings, &QAction::triggered, m_system, &QDialog::show);

}

void MainWindow::findComPort()
{
    ui->portComboBox->clear();

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
       ui->portComboBox->addItem(info.portName());
}

void MainWindow::on_connectType_currentIndexChanged(int index)
{
    if (modbusDevice) {
        modbusDevice->disconnectDevice();
        delete modbusDevice;
        modbusDevice = nullptr;
    }

    auto type = static_cast<ModbusConnection> (index);
    if (type == Serial) {
        //ui->portEdit->clear();
        modbusDevice = new QModbusRtuSerialMaster(this);
    } else if (type == Tcp) {
        modbusDevice = new QModbusTcpClient(this);
        //if (ui->portEdit->text().isEmpty())
            //ui->portEdit->setText(QLatin1Literal("127.0.0.1:502"));
    }

    connect(modbusDevice, &QModbusClient::errorOccurred, [this](QModbusDevice::Error) {
        statusBar()->showMessage(modbusDevice->errorString(), 5000);
    });

    if (!modbusDevice) {
        ui->connectButton->setDisabled(true);
        if (type == Serial)
            statusBar()->showMessage(tr("Could not create Modbus master."), 5000);
        else
            statusBar()->showMessage(tr("Could not create Modbus client."), 5000);
    } else {
        connect(modbusDevice, &QModbusClient::stateChanged,
                this, &MainWindow::onStateChanged);
    }
}

void MainWindow::modelNameReadReady()
{
    auto reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

        if (reply->error() == QModbusDevice::NoError) {
            const QModbusDataUnit unit = reply->result();
            QString s;
            for (uint i = 0; i < unit.valueCount(); i++) {
                s[2*i] = unit.value(i) >> 8;
                s[(2*i) +1] = unit.value(i) & 0x00ff;
            }
            ui->portEdit_3->setText(s);
            statusBar()->showMessage(tr("OK!"));
        } else if (reply->error() == QModbusDevice::ProtocolError) {
            statusBar()->showMessage(tr("Read response error: %1 (Mobus exception: 0x%2)").
                                        arg(reply->errorString()).
                                        arg(reply->rawResult().exceptionCode(), -1, 16), 5000);
        } else {
           // statusBar()->showMessage(tr("Read response error: %1 (code: 0x%2)").
           //                             arg(reply->errorString()).
           //                             arg(reply->error(), -1, 16), 5000);
            statusBar()->showMessage(tr("Unknown Device: Please input the correct type of IOT Devices."));
            ui->portEdit_3->setText("Unknown Device");
        }
        reply->deleteLater();
}

void MainWindow::setModelName() const
{
    ui->portEdit_3->text().clear();
    quint16 ADDR = ModelNameAddr;

    QModbusDataUnit readUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, ADDR, ModelNameEntires);
    //modbusDevice->setNumberOfRetries(1);
    if (auto *reply = modbusDevice->sendReadRequest(readUnit, ui->serverEdit->value())) {
        if (!reply->isFinished())
            connect(reply, &QModbusReply::finished, this, &MainWindow::modelNameReadReady);
        else
            delete reply; // broadcast replies return immediately
    } else {
        statusBar()->showMessage(tr("Read error: ") + modbusDevice->errorString(), 5000);
    }
}

void MainWindow::setIOChannel()
{
    _sleep(3000);
    QString modelName = ui->portEdit_3->text();

    ui->currentInputComboBox->clear();
    ui->currentOutputComboBox->clear();
    ui->voltageInputComboBox->clear();
    ui->voltageOutputcomboBox->clear();
    ui->PWMInputComboBox->clear();
    ui->PWMOutputComboBox->clear();
    ui->thermocoupleComboBox->clear();

    if (modelName.contains("LC144")) {
        ui->currentInputComboBox->addItem("1");
        ui->currentInputComboBox->addItem("3");

        ui->currentOutputComboBox->addItem("4");

        ui->voltageInputComboBox->addItem("0");
        ui->voltageInputComboBox->addItem("2");

        ui->voltageOutputcomboBox->addItem("5");

        ui->PWMOutputComboBox->addItem("6");
        ui->PWMOutputComboBox->addItem("7");
    }
    else if (modelName.contains("Un")){
        // no channel IO
    }
}

void MainWindow::on_connectButton_clicked()
{
    if (!modbusDevice)
        return;

    statusBar()->clearMessage();
    if (modbusDevice->state() != QModbusDevice::ConnectedState) {
        if (static_cast<ModbusConnection> (ui->connectType->currentIndex()) == Serial) {
            modbusDevice->setConnectionParameter(QModbusDevice::SerialPortNameParameter,
                ui->portComboBox->currentText());
            modbusDevice->setConnectionParameter(QModbusDevice::SerialParityParameter,
                m_settingsDialog->settings().parity);
            modbusDevice->setConnectionParameter(QModbusDevice::SerialBaudRateParameter,
                m_settingsDialog->settings().baud);
            modbusDevice->setConnectionParameter(QModbusDevice::SerialDataBitsParameter,
                m_settingsDialog->settings().dataBits);
            modbusDevice->setConnectionParameter(QModbusDevice::SerialStopBitsParameter,
                m_settingsDialog->settings().stopBits);
        } else {
            /*
            const QUrl url = QUrl::fromUserInput(ui->portEdit->text());
            modbusDevice->setConnectionParameter(QModbusDevice::NetworkPortParameter, url.port());
            modbusDevice->setConnectionParameter(QModbusDevice::NetworkAddressParameter, url.host());
            */
        }

        modbusDevice->setTimeout(m_settingsDialog->settings().responseTime);
        modbusDevice->setNumberOfRetries(m_settingsDialog->settings().numberOfRetries);
        if (!modbusDevice->connectDevice()) {
            statusBar()->showMessage(tr("Connect failed: ") + modbusDevice->errorString(), 5000);
        } else {
            ui->actionConnect->setEnabled(false);
            ui->actionDisconnect->setEnabled(true);
        }
    } else {
        modbusDevice->disconnectDevice();
        ui->actionConnect->setEnabled(true);
        ui->actionDisconnect->setEnabled(false);
        ui->portEdit_3->clear();
    }

    setModelName();
    setIOChannel();
}

void MainWindow::onStateChanged(int state)
{
    bool connected = (state != QModbusDevice::UnconnectedState);
    connected = connected;
    ui->actionConnect->setEnabled(!connected);
    ui->actionDisconnect->setEnabled(connected);

    if (state == QModbusDevice::UnconnectedState)
        ui->connectButton->setText(tr("Connect"));
    else if (state == QModbusDevice::ConnectedState)
        ui->connectButton->setText(tr("Disconnect"));
}

/*
    IO Set Code Configuration supposed to be Product Utility Tool
    Remain Since already done
*/
void MainWindow::writeSingleHoldingRegister(QModbusDataUnit &writeUnit_)
{
    if (auto *reply = modbusDevice->sendWriteRequest(writeUnit_, ui->serverEdit->value())) {
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

void MainWindow::_sleep(unsigned int msec)
{
    QTime reachTime = QTime::currentTime().addMSecs(msec);
    while( QTime::currentTime() < reachTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}
