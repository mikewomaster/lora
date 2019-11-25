#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QModbusDataUnit>
#include <QModbusTcpClient>
#include <QModbusRtuSerialMaster>
#include <qmodbusdevice.h>
#include <QMessageBox>
#include <stdlib.h>
#include <QTime>
#include <QPalette>
#include "ui_logindialog.h"
#include "logindialog.h"
#include "commanhelper.h"

logindialog::logindialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::logindialog),
    modbusDeviceLogin(nullptr)
{
    ui->setupUi(this);

    // hide ?
    Qt::WindowFlags flags=Qt::Dialog;
    flags |=Qt::WindowCloseButtonHint;
    setWindowFlags(flags);

    setTabOrder(ui->usernameLineEdit, ui->passwordLineEdit);

    ui->usernameLineEdit->setPlaceholderText("Username");
    ui->passwordLineEdit->setEchoMode(QLineEdit::Password);
    ui->passwordLineEdit->setPlaceholderText("Password");

    ui->serialComBox->clear();

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
       ui->serialComBox->addItem(info.portName());
}

logindialog::~logindialog()
{
    delete ui;
}

void logindialog::ReadReady()
{
    auto reply = qobject_cast<QModbusReply *>(sender());
    if (!reply) {
        m_username = "";
        return;
    }

    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();
        QString s;
        for (uint i = 0; i < 8; i++) {
            s[2*i] = unit.value(i) >> 8;
            s[(2*i) +1] = unit.value(i) & 0x00ff;
        }
        m_username = s;

        s.clear();
        for (uint i = 0; i < 8; i++) {
            s[2*i] = unit.value(i+8) >> 8;
            s[(2*i) +1] = unit.value(i+8) & 0x00ff;
        }
        m_password = s;
    } else if (reply->error() == QModbusDevice::ProtocolError) {
        m_username = "";
        m_password = "";
    } else {
        m_username = "";
        m_password = "";
    }
    reply->deleteLater();
}

void logindialog::on_loginPushButton_clicked()
{
    modbusDeviceLogin = new QModbusRtuSerialMaster();
    modbusDeviceLogin->setConnectionParameter(QModbusDevice::SerialPortNameParameter, ui->serialComBox->currentText());
    modbusDeviceLogin->setConnectionParameter(QModbusDevice::SerialBaudRateParameter,QSerialPort::Baud9600);
    modbusDeviceLogin->setConnectionParameter(QModbusDevice::SerialDataBitsParameter,QSerialPort::Data8);
    modbusDeviceLogin->setConnectionParameter(QModbusDevice::SerialParityParameter,QSerialPort::NoParity);
    modbusDeviceLogin->setConnectionParameter(QModbusDevice::SerialStopBitsParameter,QSerialPort::OneStop);
    modbusDeviceLogin->connectDevice();
    QString name = ui->usernameLineEdit->text();
    QString password = ui->passwordLineEdit->text();

    if (modbusDeviceLogin == nullptr)
        QMessageBox::information(NULL, "Login", "Can not connect to device!");

    QModbusDataUnit readUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, USERNAME, Entries*2);

    if (auto *reply = modbusDeviceLogin->sendReadRequest(readUnit, 1)) {
        if (!reply->isFinished())
            connect(reply, &QModbusReply::finished, this, ReadReady);
        else
            delete reply; // broadcast replies return immediately
    } else {
        QMessageBox::information(NULL, "Login", "Can not connect to device!");
    }

    ui->loginPushButton->setEnabled(false);
    CommanHelper::sleep(1000);

    if (m_username.contains(name) && name != "" && m_username != "") {
        ui->loginPushButton->setEnabled(true);
        if (password != "" && m_password.contains(password) && m_password != "") {
            m_password = "";
            m_username = "";
            if (modbusDeviceLogin)
                modbusDeviceLogin->disconnect();
            delete modbusDeviceLogin;
            accept();
        } else {
             QMessageBox::information(NULL, "Login", "Wrong Username and Password. \n\r\r\r\r\rPlease try again!");
             if (modbusDeviceLogin)
                 modbusDeviceLogin->disconnect();
             delete modbusDeviceLogin;
             ui->loginPushButton->setEnabled(true);
        }
    }else{
        QMessageBox::information(NULL, "Login", "Wrong Username and Password. \n\r\r\r\r\rPlease try again!");
        if (modbusDeviceLogin)
            modbusDeviceLogin->disconnect();
        delete modbusDeviceLogin;
        ui->loginPushButton->setEnabled(true);
    }
}

void logindialog::on_quitPushButton_clicked()
{
    close();
}
