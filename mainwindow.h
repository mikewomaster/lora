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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QModbusDataUnit>
#include "ui_mainwindow.h"
#include <QStandardItemModel>
#include <QtSerialPort/QtSerialPort>
#include <QTime>

// MACRO CRONTROL Area
// #define TEST_DATA
#define SKIN
QT_BEGIN_NAMESPACE

class QModbusClient;
class QModbusReply;

namespace Ui {
class MainWindow;
class SettingsDialog;
class logdialog;
}

QT_END_NAMESPACE

class SettingsDialog;
class logdialog;
class WriteRegisterModel;

#define LoraModbusStartAddr 21
#define LoraModbusEntries 7

#define currentInputAddr (1001 - 1)
#define voltageInputAddr (1017 - 1)
#define PWMInputAddr (1033 - 1)
#define thermocoupleAddr (1049 - 1)
#define currentOutputAddr (1065 - 1)
#define voltageOutputAddr (1081 - 1)
#define PWMOutputAddr (1097 - 1)
#define IOEntries 1
#define PWMIOEntries 2

#define SerialStartAddr (1 - 1)
#define SerialEntries 5

#define ModelNameAddr (134 - 1)
#define ModelNameEntires 8

#define SrcIDAddr (101 - 1)
#define SrcIDEntries 1

#define BitMapAddr (102 - 1)
#define BitMapAddrEntries 16

#define AESEnableAddr (118 - 1)
#define AESEnableEntries 1

#define AESKeyAddr (119 - 1)
#define AESKeyEntries 8

#define TimeOutTimeAddr (150 - 1)
#define TimeOutTimeEntries 1

#define RSSIADDR (161 -1)
#define SNRAddr (162 - 1)
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void initActions();
    QModbusDataUnit readRequest() const;
    QModbusDataUnit writeRequest() const;
    void writeSingleHoldingRegister(QModbusDataUnit &);
    void _sleep(unsigned int);
    void setModelName() const;
    void setIOChannel();
    void handle_write(QLineEdit* , quint16);
    void handle_read(int addr, void (MainWindow::*fp)());
    void handle_read(int addr, int entry, void (MainWindow::*fp)());
    void handle_read_ready(QLineEdit* );
    void nb_handle_write(QLineEdit*, int, int);
    void nb_handle_read_ready(QLineEdit* );

private slots:
    void on_connectButton_clicked();
    void onStateChanged(int state);
    void modelNameReadReady();
    void currentReadReady();
    void voltageReadReady();
    void PWMReadReady();
    void thermocoupleReadReady();
    void currentOutputReadReady();
    void voltageOutputReadReady();
    void PWMOutputReadReady();
    void PIDReadReady();
    void loraReadReady();
    void netIdReadReady();
    void serialReadReady();
    void on_connectType_currentIndexChanged(int);
    void on_loraSetButton_clicked();
    void on_loraGetButton_clicked();
    void findComPort();
    void on_currentOutputPushButton_clicked();
    void on_voltageOutputPushButton_clicked();
    void on_PWMOutputPushButton_clicked();
    void on_currentInputPushButton_clicked();
    void on_voltageInputPushButton_clicked();
    void on_PWMInputPushButton_clicked();
    void on_thermocoupleInputPushButton_clicked();
    void on_PwmOutputReadPushButton_clicked();
    void on_currentOutputReadPushButton_clicked();
    void on_rvPshBtn_clicked();
    void on_serialBtn_clicked();
    void on_serialBtnRead_clicked();
    void on_pidButtonRead_clicked();
    void on_pidButtonWrite_clicked();
    void on_netSIDRead_clicked();
    void on_netSIDWrite_clicked();
    void aesEnableReadReady();
    void on_netAesEnableRead_clicked();
    void on_netAesEnableWrite_clicked();
    void netTimeoutReadReady();
    void on_netTimeoutRead_clicked();
    void on_netTimeoutWrite_clicked();
    void RSSIReadReady();
    void SNRReadReady();
    void on_RSSIRead_clicked();
    void on_SNNRRead_clicked();
    void aesKeyReadReady();
    void on_netAesKeyRead_clicked();
    void on_netAesKeyWrite_clicked();
    void bitMapReadReady();
    void on_netBitMapRead_clicked();
    void on_netBitMapWrite_clicked();
    void on_aesComboBox_currentIndexChanged(int index);
    void on_abpDevAddrWrite_clicked();
    void on_abpDevAddrRead_clicked();
    void abpDevAddrReadReady();
    void on_abpAppsKeyWrite_clicked();
    void on_abpAppsKeyRead_clicked();
    void abpAppsKeyReadReady();

    void on_abpNwksKeyWrite_clicked();
    void on_abpNwksKeyRead_clicked();
    void abpNwksKeyReadReady();

    void on_otaaDevEuiWrite_clicked();
    void on_otaaDevEuiRead_clicked();
    void otaaDevEuiReadReady();

    void on_otaaAppEuiWrite_clicked();
    void on_otaaAppEuiRead_clicked();
    void otaaAppEuiReadReady();

    void on_otaaAppKeyWrite_clicked();
    void on_otaaAppKeyRead_clicked();
    void otaaAppKeyReadReady();

    void on_clearViewBtn_clicked();

    void on_mbusPrimaryRead_12_clicked();

    void on_mbusPrimaryWrite_11_clicked();

    void mbusPrimaryReadReady();

    void on_mbusSecondaryWrite_clicked();

    void on_mbusSecondaryRead_clicked();

    void mbusSecReadReady();

    void mbusReadoutReadReady();
    void on_mbusReadoutWrite_clicked();

    void on_mbusReadoutRead_clicked();

    void mbusTimeStampReadReady();
    void on_mbusTSWrite_clicked();

    void on_mbusTSRead_clicked();

    void mbusReadDeviceReadReady();
    void mbusReadValueReadReady();
    void on_mbusPrimaryRead_clicked();

    // mbus cmd tab button
    void on_pushButton_6_clicked();
    void on_pushButton_11_clicked();
    void on_pushButton_7_clicked();
    void mbusCheckResultReady();

    void apnReadReady();
    void on_apnRead_clicked();
    void on_apnWrite_clicked();

    void on_SNPushButton_clicked();
    void SNReadReady();
    void on_vinWrite_clicked();

    void on_vinRead_clicked();
    void vinRefReadReady();
    void on_voutWrite_clicked();

    void on_voutRead_clicked();
    void voutRefReadReady();
    void on_iinWrite_clicked();

    void on_iinRead_clicked();
    void iinRefReadReady();

    void on_ioutRead_clicked();
    void ioutReadReady();
    void on_ioutWrite_clicked();

    void on_factor1Write_clicked();

    void on_factor1Read_clicked();
    void factor1ReadReady();
    void on_factor2Write_clicked();

    void on_factor2Read_clicked();
    void factor2ReadReady();
    void on_factor3Write_clicked();

    void on_factor3Read_clicked();
    void factor3ReadReady();
    void on_factor4Write_clicked();

    void on_factor4Read_clicked();
    void factor4ReadReady();
    void on_factor5Write_clicked();

    void on_factor5Read_clicked();
    void factor5ReadReady();
    void on_factor6Write_clicked();

    void on_factor6Read_clicked();
    void factor6ReadReady();

    void userReadReady();
    void on_userRead_clicked();
    void on_userWrite_clicked();

    void on_passwordWrite_clicked();
    void passwordReadReady();
    void on_passwordRead_clicked();

    void on_ipWrite_clicked();
    void ipReadReady();
    void on_ipRead_clicked();

    void on_nbStatusWrite_clicked();
    void statusReadReady();
    void on_nbStatusRead_clicked();

    void on_srvWrite_clicked();
    void srvReadReady();
    void on_srvRead_clicked();

    void on_topicWrite_clicked();
    void topicReadReady();
    void on_topicRead_clicked();



    void on_portWrite_clicked();
    void portReadReady();
    void on_portRead_clicked();

    void on_idWrite_clicked();
    void idReadReady();
    void on_idRead_clicked();

    void on_mqttStatuWrite_clicked();
    void mqttStatuReadReady();
    void on_mqttStatusRead_clicked();

    void on_SNPushButtonWrite_clicked();

    void on_pidButtonWrite_8_clicked();
    void intervalReadReady();
    void on_intervalRead_clicked();

    void on_idWrite_2_clicked();

private:
    Ui::MainWindow *ui;
    QModbusReply *lastRequest;
    QModbusClient *modbusDevice;
    SettingsDialog *m_settingsDialog;
    logdialog *m_logdialog;
    WriteRegisterModel *writeModel;
    QStandardItemModel *m_Model;

    QSerialPort *m_serial;

    QList<QStandardItem *> storageItems;
};

#endif // MAINWINDOW_H
