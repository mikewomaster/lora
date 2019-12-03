#include "system.h"
#include "ui_system.h"
#include "mainwindow.h"
#include "commanhelper.h"

#include <QModbusDataUnit>
#include <QModbusTcpClient>
#include <QModbusRtuSerialMaster>
#include <QDebug>
#include <QMessageBox>
#include <stdlib.h>

systemDialog::systemDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::system)
{
    ui->setupUi(this);
    Qt::WindowFlags flags=Qt::Dialog;
    flags |=Qt::WindowCloseButtonHint;
    setWindowFlags(flags);

#ifdef MASTER
    ui->gridLayout->setEnabled(true);
    ui->usernameLineEdit->setEnabled(true);
    ui->passwordLineEdit->setEnabled(true);
#else

    /*
    ui->gridLayout->setEnabled(false);
    ui->systemApply->hide();
    ui->systemReload->hide();
    ui->passwordLabel->hide();
    ui->passwordLineEdit->hide();
    ui->usernameLabel->hide();
    ui->usernameLineEdit->hide();
    */

    ui->systemApply->setEnabled(false);
#endif
}

systemDialog::~systemDialog()
{
    delete ui;
}

void systemDialog::versionReadReady()
{
    auto reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();
        QString s;

        for (uint i = 0; i < unit.valueCount(); i++) {
            s[2*i] = unit.value(i) >> 8;
            if (s[2*i] == 0x00)
                break;
            s[(2*i) +1] = unit.value(i) & 0x00ff;
        }
        ui->systemVersionLineEdit->setText(s);
    } else if (reply->error() == QModbusDevice::ProtocolError) {

    } else {

    }
    reply->deleteLater();
}

void systemDialog::on_systemCheck_clicked()
{
    MainWindow *w = (MainWindow*) parentWidget();
    QModbusClient* modbusDevice = w->getModbusDevice();

    if (!modbusDevice)
        return;

    quint16 ADDR = VersionAddress;

    QModbusDataUnit readUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, ADDR, VersionEntries);

    if (auto *reply = modbusDevice->sendReadRequest(readUnit, 1)) {
        if (!reply->isFinished())
            connect(reply, &QModbusReply::finished, this, &systemDialog::versionReadReady);
        else
            delete reply; // broadcast replies return immediately
    } else {

    }
}

void systemDialog::on_systemSet_clicked()
{
    MainWindow *w = (MainWindow*) parentWidget();
    QModbusClient* modbusDevice = w->getModbusDevice();

    if (!modbusDevice)
        return;

    QModbusDataUnit writeUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, ResetEnableAddress, ResetEntries);
    quint16 currentOutputValue =  ui->systemReloadComboBox->currentIndex();

    writeUnit.setValue(0, currentOutputValue);
    if (auto *reply = modbusDevice->sendWriteRequest(writeUnit, 1)) {
        if (!reply->isFinished()) {
            connect(reply, &QModbusReply::finished, this, [this, reply]() {
                if (reply->error() == QModbusDevice::ProtocolError) {

                } else if (reply->error() != QModbusDevice::NoError) {

                }
                reply->deleteLater();
            });
        } else {
            // broadcast replies return immediately
            reply->deleteLater();
        }
    } else {

    }
}

void systemDialog::passwordReadReady()
{
    handle_readready(ui->passwordLineEdit);
}

void systemDialog::usernameReadReady()
{
    handle_readready(ui->usernameLineEdit);
}

void systemDialog::on_systemReload_clicked()
{
    ui->systemReload->setEnabled(false);
    ui->passwordLineEdit->clear();
    ui->usernameLineEdit->clear();
    handle_read(UsernameAddress, UsernameEntries, usernameReadReady);
    handle_read(PasswordAddress, PasswordEntries, passwordReadReady);
    CommanHelper::sleep(3000);
    ui->usernameLineEdit->setText(m_username);
    ui->passwordLineEdit->setText(m_password);
    ui->systemReload->setEnabled(true);
}

void systemDialog::handle_readready(QLineEdit *le)
{
    auto reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();
        QString s;
        for (uint i = 0; i < unit.valueCount(); i++) {
            if ((unit.value(i) >> 8) == 0x00)
                break;
            s[2*i] = unit.value(i) >> 8;

            if ((unit.value(i) & 0x00ff) == 0x00)
                break;
            s[(2*i) +1] = unit.value(i) & 0x00ff;
        }
        if (le == ui->usernameLineEdit)
            m_username = s;
        if (le == ui->passwordLineEdit)
            m_password = s;
        // le->setText(s);
    } else if (reply->error() == QModbusDevice::ProtocolError) {

    } else {

    }
    reply->deleteLater();
}

void systemDialog::handle_read(int addr, int entry, void (systemDialog::*fp)())
{
    MainWindow *w = (MainWindow*) parentWidget();
    QModbusClient* modbusDevice = w->getModbusDevice();

    if (!modbusDevice)
        return;

    QModbusDataUnit readUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, addr, entry);

    if (auto *reply = modbusDevice->sendReadRequest(readUnit, 1)) {
        if (!reply->isFinished())
            connect(reply, &QModbusReply::finished, this, fp);
        else
            delete reply; // broadcast replies return immediately
    } else {

    }
}

void systemDialog::on_systemApply_clicked()
{
    ui->systemApply->setEnabled(false);
    // change write twice to 1 time
    //handle_write(ui->usernameLineEdit, UsernameAddress, UsernameEntries);
    //CommanHelper::sleep(2000);
    //handle_write(ui->passwordLineEdit, PasswordAddress, PasswordEntries);

    handle_write(UsernameAddress, UsernameEntries+PasswordEntries);
    ui->systemApply->setEnabled(true);
}

void systemDialog::handle_write(int addr, int entry)
{
    QString str1 = ui->usernameLineEdit->text();
    QString str2 = ui->passwordLineEdit->text();
    QVector<quint16> values;

    int i = 0;

    // username
    for (i = 0; i < str1.size(); i++) {
        if ((i+1) % 2 == 0) {
            quint16 temp = str1.at(i - 1).toLatin1();
            temp = (temp << 8) + str1.at(i).toLatin1();
            values.push_back(temp);
        }
    }

    if (i % 2) {
        quint16 temp = str1.at(i-1).toLatin1();
        temp = temp << 8;
        values.push_back(temp);
        i++;
    }

    for (i = (i / 2); i < (entry/2); i++) {
        values.push_back(0x0000);
    }
    // password
    for (i = 0; i < str2.size(); i++) {
        if ((i+1) % 2 == 0) {
            quint16 temp = str2.at(i - 1).toLatin1();
            temp = (temp << 8) + str2.at(i).toLatin1();
            values.push_back(temp);
        }
    }

    if (i % 2) {
        quint16 temp = str2.at(i-1).toLatin1();
        temp = temp << 8;
        values.push_back(temp);
        i++;
    }

    for (i = (i / 2); i < (entry/2); i++) {
        values.push_back(0x0000);
    }

    MainWindow *w = (MainWindow*) parentWidget();
    QModbusClient* modbusDevice = w->getModbusDevice();

    if (!modbusDevice)
        return;

    QModbusDataUnit writeUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, addr, entry);

    writeUnit.setValues(values);
    if (auto *reply = modbusDevice->sendWriteRequest(writeUnit, 1)) {
        if (!reply->isFinished()) {
            connect(reply, &QModbusReply::finished, this, [this, reply]() {
                if (reply->error() == QModbusDevice::ProtocolError) {

                } else if (reply->error() != QModbusDevice::NoError) {

                }

                reply->deleteLater();
            });
        } else {
            // broadcast replies return immediately
            reply->deleteLater();
        }
    } else {

    }
}

void systemDialog::SNReadReady()
{
    auto reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();
        QString s;
        for (uint i = 0; i < unit.valueCount(); i++) {
            if ((unit.value(i) >> 8) == 0x00)
                break;
            s[2*i] = unit.value(i) >> 8;

            if ((unit.value(i) & 0x00ff == 0x00)) {
                break;
            }
            s[(2*i) +1] = unit.value(i) & 0x00ff;
        }
        ui->SNShow->setText(s);
    } else if (reply->error() == QModbusDevice::ProtocolError) {

    } else {

    }
    reply->deleteLater();
}

void systemDialog::on_SNRead_clicked()
{
    MainWindow *w = (MainWindow*) parentWidget();
    QModbusClient* modbusDevice = w->getModbusDevice();

    quint16 ADDR = SNAddr2;

    QModbusDataUnit readUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, ADDR, SNEntries2);

    if (auto *reply = modbusDevice->sendReadRequest(readUnit, 1)) {
        if (!reply->isFinished())
            connect(reply, &QModbusReply::finished, this, &systemDialog::SNReadReady);
        else
            delete reply; // broadcast replies return immediately
    } else {

    }
}
