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
#include "netmodel.h"
#include "logindialog.h"

#include <QModbusTcpClient>
#include <QModbusRtuSerialMaster>
#include <QStandardItemModel>
#include <QStatusBar>
#include <QUrl>
#include <QtSerialPort/QSerialPortInfo>
#include <QDebug>
#include <QTime>
#include <QSize>
#include <QDebug>
#include <QMessageBox>

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
    , m_login_flag(0)
    , m_login_flag_2(0)
    , frequency(0)
    , monitorAlarm(nullptr)
    , m_ymodem(new ymodem())
{
    ui->setupUi(this);

    QString title = UtilityVersion;
    setWindowTitle(title);

    m_settingsDialog = new SettingsDialog(this);
    m_logdialog = new logdialog(this);
    m_system = new systemDialog(this);
    m_sensor_dialog = new sensor_edit(this);
    m_obis_edit_dialog = new obis_edit(this);

    initActions();

    writeModel = new WriteRegisterModel(this);

    ui->connectType->setCurrentIndex(0);
    on_connectType_currentIndexChanged(0);

    sensorFlag = false;
    sensor_edit_flag = false;
    obis_edit_flag = false;

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        serialInfoVector.push_back(info.portName());
        ui->portComboBox->addItem(info.portName());
    }

    m_Model->setHorizontalHeaderItem(0, new QStandardItem(QObject::tr("Address")));
    m_Model->setHorizontalHeaderItem(1, new QStandardItem(QObject::tr("Mode")));
    m_Model->setHorizontalHeaderItem(2, new QStandardItem(QObject::tr("SerialNumber ")));
    m_Model->setHorizontalHeaderItem(3, new QStandardItem(QObject::tr("Manu")));
    m_Model->setHorizontalHeaderItem(4, new QStandardItem(QObject::tr("Version")));
    m_Model->setHorizontalHeaderItem(5, new QStandardItem(QObject::tr("Medium")));
    // m_Model->setHorizontalHeaderItem(6, new QStandardItem(QObject::tr("Type")));
    m_Model->setHorizontalHeaderItem(6, new QStandardItem(QObject::tr("Value")));
    m_Model->setHorizontalHeaderItem(7, new QStandardItem(QObject::tr("Unit")));
    // m_Model->setHorizontalHeaderItem(8, new QStandardItem(QObject::tr("Scale")));
    m_Model->setHorizontalHeaderItem(8, new QStandardItem(QObject::tr("Description")));
    m_Model->setHorizontalHeaderItem(9, new QStandardItem(QObject::tr("TimeStamp  ")));

    ui->connectType->hide();
    ui->serverEdit->hide();
    ui->syncWordLineEdit->hide();
    ui->OptimizeLowRateComboBox->hide();

    ui->tabWidget->setTabEnabled(6, false);
    // ui->tabWidget->setTabEnabled(8, false);
    // ui->tabWidget->setTabEnabled(9, false);
    ui->groupBox_6->hide();

#ifdef LORA
    ui->tabWidget->show();
    ui->tabWidget_2->hide();
#else
    ui->tabWidget_2->show();
    ui->tabWidget->hide();
#endif

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

    ui->intervalLineEdit->hide();
    ui->intervalRead->hide();
    ui->idWrite_2->hide();

    ui->mbusSecondaryEdit->hide();
    ui->mbusSecondaryRead->hide();
    ui->mbusSecondaryWrite->hide();

    // SN hide
    ui->SNLineEdit->hide();
    ui->SNPushButton->hide();
    ui->SNPushButtonWrite->hide();

    // menubar hide
    ui->menuDevice->setVisible(false);
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

    item.append(new QStandardItem(QObject::tr("3")));
    item.append(new QStandardItem(QObject::tr("primary")));
    item.append(new QStandardItem(QObject::tr("201911040001")));
    item.append(new QStandardItem(QObject::tr("WM")));
    item.append(new QStandardItem(QObject::tr("0.0.1")));
    item.append(new QStandardItem(QObject::tr("Heat")));
    item.append(new QStandardItem(QObject::tr("2 BCD")));
    item.append(new QStandardItem(QObject::tr("J/h")));
    item.append(new QStandardItem(QObject::tr("0.001")));
    item.append(new QStandardItem(QObject::tr("99.6")));
    item.append(new QStandardItem(QObject::tr("HMS Heat Downtown Man Shoot 3!")));
    m_Model->appendRow(item);
    item.clear();
#endif

    ui->tableView->setModel(m_Model);
    ui->tableView->verticalHeader()->hide();
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // resize
#if 0
    QWidget *widgetAdjust = new QWidget(this);
    setCentralWidget(widgetAdjust);
    QVBoxLayout *windowLayoutAdjust = new QVBoxLayout;
    windowLayoutAdjust->addLayout(ui->gridLayout, 1);
    windowLayoutAdjust->addWidget(ui->tabWidget_2);
    widgetAdjust->setLayout(windowLayoutAdjust);
#endif

    // sensor Check Push Button
    ui->sensorCheckPushButton->setVisible(false);
    ui->sensorTypeCombox->setVisible(false);

    // nb
    ui->nbModelRead->hide();
    ui->nbModeWrite->hide();

    ui->BitMapTextEdit->hide();

    // group net table view
    groupNetViewModel();

    ui->coapInterval->hide();
    ui->dlmsPwd->setEnabled(false);

    // dlms table view with dlmsModel
    obis_view_model_init();

    // sensor view and sensor model init
    sensor_view_model();
    ruleChainViewModel();
    ruleChainMonitorViewModel();
    eventMVC();

    serialAlarmInit();
    for (int i = 1; i <= 250; i++ ) {
        ui->ruleInDeviceIDComboBox->addItem(QString::number(i));
        ui->ruleOutIDComboBox->addItem(QString::number(i));
    }

    for (int i = 0; i < 8; i++) {
        ui->ruleInChComboBox->addItem(QString::number(i));
        ui->ruleOutChnComboBox->addItem(QString::number(i));
    }

    // insertValueModule();
    // insertMonitorValueModule();

    ui->ELogChkPushButton->setEnabled(false);
}

MainWindow::~MainWindow()
{
    if (modbusDevice)
        modbusDevice->disconnectDevice();
    delete modbusDevice;

    delete ui;
}

void MainWindow::eventMVC()
{
    eLModel = new eventLogModel(this);

    ui->EventLogView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->EventLogView->setShowGrid(true);
    ui->EventLogView->setFrameShape(QFrame::Box);
    ui->EventLogView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->EventLogView->setModel(eLModel);
    ui->EventLogView->setSortingEnabled(true);
    ui->EventLogView->setColumnWidth(0, 170);
    ui->EventLogView->setColumnWidth(1, 170);
    ui->EventLogView->setColumnWidth(2, 100);
    ui->EventLogView->setColumnWidth(3, 170);
    ui->EventLogView->setColumnWidth(4, 170);
    ui->EventLogView->setColumnWidth(5, 230);
    ui->EventLogView->setContextMenuPolicy(Qt::CustomContextMenu);

    eventLogList.clear();
    eventLogData record;
    for (int i = 0; i < 1000; i++) {
        record.tag = "";
        record.valueInput = 0;
        record.type = 0;
        record.valueOutput = 0;
        record.timestamp = 0;
        record.res = 0;
        record.sequence = i;
        eventLogList.append(record);
    }
    eLModel->updateData(eventLogList);

    for (int i = 0; i < 1000; i++) {
        if (record.tag == "")
            ui->EventLogView->setRowHidden(i, true);
    }
}

void MainWindow::ruleChainMonitorViewModel()
{
    m_RuleMonitorModel = new ruleMonitor(this);

    ui->MonitorView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->MonitorView->setShowGrid(true);
    ui->MonitorView->setFrameShape(QFrame::Box);
    ui->MonitorView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->MonitorView->setModel(m_RuleMonitorModel);
    ui->MonitorView->setColumnWidth(0, 170);
    ui->MonitorView->setColumnWidth(1, 170);
    ui->MonitorView->setColumnWidth(2, 170);
    ui->MonitorView->setColumnWidth(3, 170);
    ui->MonitorView->setColumnWidth(4, 230);
    ui->MonitorView->setContextMenuPolicy(Qt::CustomContextMenu);

    ruleMonitorList.clear();
    ruleMonitorStruct record;
    for (int i = 0; i < 40; i++) {
        record.tag = "";
        record.value = 0;
        record.type = 0;
        record.result = 0;
        record.timestamp = 0;
        record.ts = "";
        record.sequence = i;
        ruleMonitorList.append(record);
    }
    m_RuleMonitorModel->updateData(ruleMonitorList);

    for (int i = 0; i < 40; i++) {
        if (record.tag == "")
            ui->MonitorView->setRowHidden(i, true);
    }
    ui->MonitorView->setColumnHidden(5, true);
}

void MainWindow::asMenuPaint(QPoint pos)
{
    asMenu->exec(QCursor::pos());
}

void MainWindow::asSettingAction()
{
    m_asDialog = new asDialog(this);
    m_asDialog->setWindowTitle("ID-Reg Settings");
    m_asDialog->show();
}

void MainWindow::groupNetViewModel()
{
    m_pModel = new NetModel(nullptr);
    ui->netBitMapTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->netBitMapTableView->setShowGrid(false);
    ui->netBitMapTableView->setFrameShape(QFrame::NoFrame);
    ui->netBitMapTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->netBitMapTableView->setModel(m_pModel);
    ui->netBitMapTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QList<Device> recordList;
    for (int i = 1; i <= 250; ++i)
    {
        Device record;
        record.bChecked = false;
        record.id = i;
        recordList.append(record);
    }
    m_pModel->updateData(recordList);

    ui->netBitMapTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    asMenu = new QMenu(ui->netBitMapTableView);
    QAction *actionSetting = new QAction();
    actionSetting->setText(QString("Settings"));
    asMenu->addAction(actionSetting);
    connect(ui->netBitMapTableView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(asMenuPaint(QPoint)));
    connect(actionSetting, &QAction::triggered, this, &MainWindow::asSettingAction);
}

void MainWindow::ruleChainViewModel()
{
    // ui->monitorRadioButton->setHidden(true);

    m_ruleChainModel = new RuleChain(this);

    ui->RuleVIew->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->RuleVIew->setShowGrid(true);
    ui->RuleVIew->setFrameShape(QFrame::Box);
    ui->RuleVIew->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->RuleVIew->setModel(m_ruleChainModel);
    ui->RuleVIew->setColumnWidth(0, 100);
    ui->RuleVIew->setColumnWidth(1, 85);
    ui->RuleVIew->setColumnWidth(2, 70);
    ui->RuleVIew->setColumnWidth(3, 100);
    ui->RuleVIew->setColumnWidth(4, 100);
    ui->RuleVIew->setColumnWidth(5, 80);
    ui->RuleVIew->setContextMenuPolicy(Qt::CustomContextMenu);

    ruleChainMenu = new QMenu(ui->RuleVIew);
    QAction *actionUpdateSensInfo = new QAction();
    QAction *actionDelSensInfo = new QAction();
    actionUpdateSensInfo ->setText(QString("Edit"));
    actionDelSensInfo ->setText(QString("Delete"));
    ruleChainMenu->addAction(actionUpdateSensInfo);
    ruleChainMenu->addAction(actionDelSensInfo);
    connect(ui->RuleVIew, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(ruleChainMenuPaint(QPoint)));

    connect(actionUpdateSensInfo, &QAction::triggered, this, &MainWindow::ruleChainEditAction);
    connect(actionDelSensInfo, &QAction::triggered, this, &MainWindow::ruleChainDelete);

    ruleChainList.clear();
    rule_chain record;
    for (int i = 0; i < 40; i++) {
        record.ruleName = "";
        record.inDevId = 0;
        record.inDevCh = 0;
        record.ruleType = 0;
        record.outDevId = 0;
        record.outDevCh = 0;
        record.seq = i;
        ruleChainList.append(record);
    }
    m_ruleChainModel->updateData(ruleChainList);

    for (int i = 0; i < 40; i++) {
        if (record.ruleName == "")
            ui->RuleVIew->setRowHidden(i, true);
    }
    ui->RuleVIew->setColumnHidden(6, true);
}

void MainWindow::insertValueModule()
{
    QStandardItemModel *ruleModel = new QStandardItemModel();

    ruleModel->setHorizontalHeaderItem(0, new QStandardItem(QObject::tr("Rule Name")));
    ruleModel->setHorizontalHeaderItem(1, new QStandardItem(QObject::tr("Input ID")));
    ruleModel->setHorizontalHeaderItem(2, new QStandardItem(QObject::tr("Address")));
    ruleModel->setHorizontalHeaderItem(3, new QStandardItem(QObject::tr("Rule Type")));
    ruleModel->setHorizontalHeaderItem(4, new QStandardItem(QObject::tr("Output ID")));
    ruleModel->setHorizontalHeaderItem(5, new QStandardItem(QObject::tr("Address")));

    QList<QStandardItem *> item;

    item.append(new QStandardItem(QObject::tr("FishLED")));
    item.append(new QStandardItem(QObject::tr("1")));
    item.append(new QStandardItem(QObject::tr("123")));
    item.append(new QStandardItem(QObject::tr("Voltage-Voltage")));
    item.append(new QStandardItem(QObject::tr("2")));
    item.append(new QStandardItem(QObject::tr("25")));
    ruleModel->appendRow(item);
    item.clear();

    item.append(new QStandardItem(QObject::tr("PLC")));
    item.append(new QStandardItem(QObject::tr("12")));
    item.append(new QStandardItem(QObject::tr("31")));
    item.append(new QStandardItem(QObject::tr("Voltage-Current")));
    item.append(new QStandardItem(QObject::tr("24")));
    item.append(new QStandardItem(QObject::tr("17")));
    ruleModel->appendRow(item);
    item.clear();

    ui->RuleVIew->setModel(ruleModel);
    ui->RuleVIew->verticalHeader()->hide();
    ui->RuleVIew->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void MainWindow::insertMonitorValueModule()
{
    QStandardItemModel *ruleModel = new QStandardItemModel();

    ruleModel->setHorizontalHeaderItem(0, new QStandardItem(QObject::tr("Rule Name")));
    ruleModel->setHorizontalHeaderItem(1, new QStandardItem(QObject::tr("Collect Salve Analog")));
    ruleModel->setHorizontalHeaderItem(2, new QStandardItem(QObject::tr("Handle")));
    ruleModel->setHorizontalHeaderItem(3, new QStandardItem(QObject::tr("Controlled Result")));
    ruleModel->setHorizontalHeaderItem(4, new QStandardItem(QObject::tr("Time Stamp")));
    QList<QStandardItem *> item;

    QDateTime time = QDateTime::currentDateTime();
    QString StrCurrentTime = time.toString("yyyy-MM-dd hh:mm:ss ddd");

    item.append(new QStandardItem(QObject::tr("FishLED")));
    item.append(new QStandardItem(QObject::tr("12")));
    item.append(new QStandardItem(QObject::tr("Voltage-Voltage")));
    item.append(new QStandardItem(QObject::tr("Success")));
    item.append(new QStandardItem(StrCurrentTime));
    ruleModel->appendRow(item);
    item.clear();

    item.append(new QStandardItem(QObject::tr("PLC")));
    item.append(new QStandardItem(QObject::tr("233")));
    item.append(new QStandardItem(QObject::tr("Voltage-Input")));
    item.append(new QStandardItem(QObject::tr("Fail")));
    item.append(new QStandardItem(StrCurrentTime));
    ruleModel->appendRow(item);
    item.clear();

    ruleModel->item(0, 0)->setTextAlignment(Qt::AlignCenter);
    ruleModel->item(0, 1)->setTextAlignment(Qt::AlignCenter);
    ruleModel->item(0, 2)->setTextAlignment(Qt::AlignCenter);
    ruleModel->item(0, 3)->setTextAlignment(Qt::AlignCenter);
    ruleModel->item(0, 4)->setTextAlignment(Qt::AlignCenter);
    ruleModel->item(1, 0)->setTextAlignment(Qt::AlignCenter);
    ruleModel->item(1, 1)->setTextAlignment(Qt::AlignCenter);
    ruleModel->item(1, 2)->setTextAlignment(Qt::AlignCenter);
    ruleModel->item(1, 3)->setTextAlignment(Qt::AlignCenter);
    ruleModel->item(1, 4)->setTextAlignment(Qt::AlignCenter);

    ui->MonitorView->setModel(ruleModel);
    ui->MonitorView->verticalHeader()->hide();
    ui->MonitorView->setColumnWidth(0, 180);
    ui->MonitorView->setColumnWidth(1, 189);
    ui->MonitorView->setColumnWidth(2, 180);
    ui->MonitorView->setColumnWidth(3, 180);
    ui->MonitorView->setColumnWidth(4, 210);
}

void MainWindow::initActions()
{

#ifdef ACTION
    ui->actionConnect->setEnabled(true); 
    ui->actionDisconnect->setEnabled(false);
    ui->actionExit->setEnabled(true);
    ui->actionOptions->setEnabled(true);

    connect(ui->actionConnect, &QAction::triggered,this, &MainWindow::on_connectButton_clicked);
    connect(ui->actionDisconnect, &QAction::triggered,this, &MainWindow::on_connectButton_clicked);
    connect(ui->actionExit, &QAction::triggered, this, &QMainWindow::close);
    connect(ui->actionOptions, &QAction::triggered, m_settingsDialog, &QDialog::show);
    //connect(ui->actionLog, &QAction::triggered, m_logdialog, &QDialog::show);
#endif

    ui->action_Settings->setEnabled(true);
    connect(ui->action_Settings, &QAction::triggered, m_system, &QDialog::show);

    ui->action_Default->setEnabled(true);
    connect(ui->action_Default, &QAction::triggered, this, &MainWindow::defaultTheme);
    ui->actionDa_rk->setEnabled(true);
    connect(ui->actionDa_rk, &QAction::triggered, this, &MainWindow::darkTheme);
    connect(m_ymodem, SIGNAL(finRcv(QString)), this, SLOT(parseEventLog(QString)));
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
    do {
        _sleep(10);
    }while(ui->portEdit_3->text() == "");
    QString modelName =  ui->portEdit_3->text();

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
    else if (modelName.contains("LR")){
        ui->currentInputComboBox->addItem("1");
        ui->currentInputComboBox->addItem("3");

        ui->voltageInputComboBox->addItem("0");
        ui->voltageInputComboBox->addItem("2");
    }
    else if (modelName.contains("Un")){
        // no channel IO
    }
}

void MainWindow::setWidgetLoRa()
{
    QString s = ui->portEdit_3->text();

    if (s.contains("LM100")) {
        ui->tabWidget->setTabEnabled(3, false);
        ui->tabWidget->setTabEnabled(4, false);
        ui->tabWidget->setTabEnabled(7, false);
        ui->tabWidget->setTabEnabled(8, false);
        ui->tabWidget->setTabEnabled(9, false);
        ui->tabWidget->setTabEnabled(10, false);
        ui->netSIDRead->setEnabled(false);
        ui->netSIDWrite->setEnabled(false);
    }else if (s.contains("LM200")) {
        ui->tabWidget->setTabEnabled(3, false);
        ui->tabWidget->setTabEnabled(4, false);
        ui->tabWidget->setTabEnabled(7, false);
        ui->netSIDRead->setEnabled(false);
        ui->netSIDWrite->setEnabled(false);
    }
    else if (s.contains("LC")) {
        ui->tabWidget->setTabEnabled(3, false);
        ui->tabWidget->setTabEnabled(7, false);
        ui->tabWidget->setTabEnabled(8, false);
        ui->tabWidget->setTabEnabled(9, false);
        ui->tabWidget->setTabEnabled(10, false);
        ui->netSIDRead->setEnabled(true);
        ui->netSIDWrite->setEnabled(true);
    }else if (s.contains("LR")) {
        ui->tabWidget->setTabEnabled(1, false);
        ui->tabWidget->setTabEnabled(2, false);
        ui->tabWidget->setTabEnabled(8, false);
        ui->tabWidget->setTabEnabled(9, false);
        ui->tabWidget->setTabEnabled(10, false);
        // ui->tabWidget->setTabEnabled(4, false);

        ui->label_8->setVisible(false);
        ui->portEdit_2->setVisible(false);
        ui->label_71->setVisible(false);
    }

    if (s.contains("400")){
        frequency = freq400;
        ui->FrequencyComboBox->addItem("433");
        ui->FrequencyComboBox->addItem("470");
        ui->FrequencyComboBox->addItem("490");
    } else if (s.contains("TH")){
        frequency = freqTH;
        ui->FrequencyComboBox->addItem("920");
        ui->FrequencyComboBox->addItem("925");
    } else if (s.contains("900")) {
        frequency = freq800;
        ui->FrequencyComboBox->addItem("868");
        ui->FrequencyComboBox->addItem("915");
    }

    ui->tabWidget->setStyleSheet("QTabBar::tab:disabled {width: 0; color: transparent;}");
    update();
}

void MainWindow::setWidget()
{
    QString s = ui->portEdit_3->text();

    if (s.contains("SCB")){
        ui->tabWidget->hide();
        ui->tabWidget_2->show();
    }else if (s.contains("DL")){
        ui->tabWidget_2->setTabEnabled(4, false);
        ui->tabWidget_2->setTabEnabled(5, false);
        ui->tabWidget_2->setTabEnabled(6, false);
    }

    ui->tabWidget_2->setTabEnabled(4, false);
    ui->tabWidget_2->setTabEnabled(5, false);
    ui->tabWidget_2->setTabEnabled(6, false);

    ui->tabWidget_2->setStyleSheet("QTabBar::tab:disabled {width: 0; color: transparent;}");

    update();
}

void MainWindow::on_connectButton_clicked()
{
    if (!modbusDevice)
        return;

    statusBar()->clearMessage();

    if (modbusDevice->state() == QModbusDevice::ConnectedState) {
            modbusDevice->disconnectDevice();
            ui->actionConnect->setEnabled(true);
            ui->actionDisconnect->setEnabled(false);         
            ui->label_8->setVisible(true);
            ui->portEdit_2->setVisible(true);
            ui->label_71->setVisible(true);

            QString s = ui->portEdit_3->text();
            if (s.contains("LM100")) {
                ui->tabWidget->setTabEnabled(3, true);
                ui->tabWidget->setTabEnabled(4, true);
                ui->tabWidget->setTabEnabled(8, true);
                ui->tabWidget->setTabEnabled(9, true);
                ui->tabWidget->setTabEnabled(10, true);
            }else if (s.contains("LM200")) {
                ui->tabWidget->setTabEnabled(3, true);
                ui->tabWidget->setTabEnabled(4, true);
            }
            else if (s.contains("LC")) {
                ui->tabWidget->setTabEnabled(3, true);
                ui->tabWidget->setTabEnabled(8, true);
                ui->tabWidget->setTabEnabled(9, true);
                ui->tabWidget->setTabEnabled(10, true);
            } else if (s.contains("LR")) {
                ui->tabWidget->setTabEnabled(1, true);
                ui->tabWidget->setTabEnabled(2, true);
                ui->tabWidget->setTabEnabled(4, true);
                ui->tabWidget->setTabEnabled(8, true);
                ui->tabWidget->setTabEnabled(9, true);
                ui->tabWidget->setTabEnabled(10, true);
            } else if (s.contains("DL")) {
                // ui->tabWidget_2->setTabEnabled(4, true);
                // ui->tabWidget_2->setTabEnabled(5, true);
                // ui->tabWidget_2->setTabEnabled(6, true);
            }

            ui->tabWidget->setStyleSheet("QTabBar::tab:disabled {width: 0; color: transparent;}");
            ui->tabWidget_2->setStyleSheet("QTabBar::tab:disabled {width: 0; color: transparent;}");
            ui->portEdit_3->clear();
            update(); // repaint

            m_login_flag = 0;
            m_login_flag_2 = 0;
            sensorFlag = false;

            ui->FrequencyComboBox->clear();
     } else {
        logindialog log(this);
        log.show();

        // init
        m_login_flag_2 = 0;

        while(!m_login_flag) {
            // quit button
            if (m_login_flag_2)
                return;
            _sleep(1500);
        }

        ui->actionConnect->setEnabled(false);
        ui->actionDisconnect->setEnabled(true);

        setModelName();
        setIOChannel();

        #ifdef LORA
            setWidgetLoRa();
        #elif
            setWidget();
        #endif
    }
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

void MainWindow::defaultTheme()
{
    QFile qss(":stylesheet.qss");
    if( qss.open(QFile::ReadOnly)) {
           QString style = QLatin1String(qss.readAll());
           this->setStyleSheet(style);
           qss.close();
    }
    ui->tabWidget->setTabEnabled(6, false);
    ui->tabWidget->setStyleSheet("QTabBar::tab:disabled {width: 0; color: transparent;}");
}

void MainWindow::darkTheme()
{
    QFile qss(":stylesheetblack.qss");
    if( qss.open(QFile::ReadOnly)) {
           QString style = QLatin1String(qss.readAll());
           this->setStyleSheet(style);
           qss.close();
    }
    ui->tabWidget->setTabEnabled(6, true);
    ui->tabWidget->setStyleSheet("QTabBar::tab:disabled {width: 0; color: transparent;}");
}

void MainWindow::on_netBitMapClear_clicked()
{
    m_pModel->clearDate();
}

void MainWindow::serialAlarmInit()
{
    serialAlarm = new QTimer();
    serialAlarm->stop();
    serialAlarm->setInterval(1000);
    connect(serialAlarm, SIGNAL(timeout()), this, SLOT(serialAlarmTask()));
    serialAlarm->start();
}

void MainWindow::serialAlarmTask()
{
    QVector<QString> tmp;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        tmp.push_back(info.portName());
    }
    if (tmp != serialInfoVector) {
        serialInfoVector.clear();
        ui->portComboBox->clear();
        foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
            serialInfoVector.push_back(info.portName());
            ui->portComboBox->addItem(info.portName());
        }
    }
}
