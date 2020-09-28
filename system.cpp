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
#include <QDialogButtonBox>

systemDialog::systemDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::system)
{
    ui->setupUi(this);
    Qt::WindowFlags flags=Qt::Dialog;
    flags |=Qt::WindowCloseButtonHint;
    setWindowFlags(flags);

    ui->systemReloadComboBox->hide();
#ifdef MASTER
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
    ui->widget->setVisible(false);
    this->setFixedSize(510, 315);
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

    QMessageBox::StandardButton result;
    result = QMessageBox::information(NULL, "Reset", "WARN: It will reset the system to default setting value.", QMessageBox::Yes | QMessageBox::No);
    if (result == QMessageBox::No)
        return;

    QModbusDataUnit writeUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, ResetEnableAddress, ResetEntries);
    // quint16 currentOutputValue =  ui->systemReloadComboBox->currentIndex();
    quint16 currentOutputValue = 1;

    writeUnit.setValue(0, currentOutputValue);
    if (auto *reply = modbusDevice->sendWriteRequest(writeUnit, 1)) {
        if (!reply->isFinished()) {
            connect(reply, &QModbusReply::finished, this, [this, reply]() {
                MainWindow *mw = (MainWindow*) parentWidget();
                if (reply->error() == QModbusDevice::ProtocolError) {

                    mw->statusBar()->showMessage(tr("Write response error: %1 (Mobus exception: 0x%2)")
                        .arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16),
                        5000);

                } else if (reply->error() != QModbusDevice::NoError) {

                    mw->statusBar()->showMessage(tr("Write response error: %1 (code: 0x%2)").
                        arg(reply->errorString()).arg(reply->error(), -1, 16), 5000);
                }
                mw->statusBar()->showMessage(tr("OK!"));
                reply->deleteLater();
            });
        } else {
            // broadcast replies return immediately
            reply->deleteLater();
        }
    } else {
        w->statusBar()->showMessage(tr("Write error: ") + modbusDevice->errorString(), 5000);
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
    // change write twice to 1 time
    //handle_write(ui->usernameLineEdit, UsernameAddress, UsernameEntries);
    //CommanHelper::sleep(2000);
    //handle_write(ui->passwordLineEdit, PasswordAddress, PasswordEntries);

    QMessageBox::StandardButton result;
    result = QMessageBox::information(NULL, "", "WARN: It will change the username and password for login.", QMessageBox::Yes | QMessageBox::No);
    if (result == QMessageBox::No) {
        return;
    }

    ui->systemApply->setEnabled(false);
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
                    QMessageBox::information(NULL, "", "Failed to Reset Username and Password.");
                } else if (reply->error() != QModbusDevice::NoError) {
                    QMessageBox::information(NULL, "", "Failed to Reset Username and Password.");
                }
                reply->deleteLater();
            });
        } else {
            QMessageBox::information(NULL, "", "Successed to Reset Username and Password.");
            // broadcast replies return immediately
            reply->deleteLater();
        }
    } else {
        // QMessageBox::information(NULL, "", "Successed to Reset Username and Password.");
    }
}

void systemDialog::SNReadReady()
{
    auto reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();

        if ((unit.value(0) >> 8 == 0xff))
                return;

        QString s;
        for (uint i = 0; i < unit.valueCount(); i++) {
            if ((unit.value(i) >> 8) == 0x00)
                break;
            s[2*i] = unit.value(i) >> 8;

            if ((unit.value(i) & 0x00ff) == 0x00) {
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

void systemDialog::on_pushButton_clicked()
{
    if (ui->pushButton->text() == ">>>") {
        this->setFixedSize(510, 481);
        ui->widget->setVisible(true);
        ui->pushButton->setText("<<<");
    } else if (ui->pushButton->text() == "<<<") {
        this->setFixedSize(510, 315);
        ui->widget->setVisible(false);
        ui->pushButton->setText(">>>");
    }
}

void systemDialog::globalTSChkReadReady()
{
    MainWindow *w = (MainWindow*) parentWidget();
    auto reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();

        quint64 data;
        data = unit.value(0);
        data = (data << 16) + unit.value(1);

        QDateTime time = QDateTime::fromTime_t(data);
        QString StrCurrentTime = time.toString("yyyy-MM-dd hh:mm:ss ddd");
        ui->globalTSLineEdit->setText(StrCurrentTime);
        w->statusBar()->showMessage(tr("OK!"));
    } else if (reply->error() == QModbusDevice::ProtocolError) {
        w->statusBar()->showMessage(tr("Read response error: %1 (Mobus exception: 0x%2)").
                                    arg(reply->errorString()).
                                    arg(reply->rawResult().exceptionCode(), -1, 16), 5000);
    } else {
        w->statusBar()->showMessage(tr("Read response error: %1 (code: 0x%2)").
                                    arg(reply->errorString()).
                                    arg(reply->error(), -1, 16), 5000);
    }
    reply->deleteLater();
}

void systemDialog::on_globalTSCheckPushButton_clicked()
{
    MainWindow *w = (MainWindow*) parentWidget();
    QModbusClient* modbusDevice = w->getModbusDevice();

    if (!modbusDevice)
        return;
    w->statusBar()->clearMessage();

    quint16 ADDR = SENSORTS;

    QModbusDataUnit readUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, ADDR, 2);

    if (auto *reply = modbusDevice->sendReadRequest(readUnit, 1)) {
        if (!reply->isFinished())
            connect(reply, &QModbusReply::finished, this, &systemDialog::globalTSChkReadReady);
        else
            delete reply; // broadcast replies return immediately
    } else {
        w->statusBar()->showMessage(tr("Read error: ") + modbusDevice->errorString(), 5000);
    }
}

void systemDialog::on_globalTSSetPushButton_clicked()
{
    MainWindow *w = (MainWindow*) parentWidget();
    QModbusClient* modbusDevice = w->getModbusDevice();

    QDateTime time = QDateTime::currentDateTime();
    int timeT = time.toTime_t();
    QString StrCurrentTime = time.toString("yyyy-MM-dd hh:mm:ss ddd");
    ui->globalTSLineEdit->setText(StrCurrentTime);

    quint32 timeoutStamp = timeT;
    QVector<quint16> values;

    for (int i = 1; i >= 0; i--) {
        quint16 temp = 0;
        temp = (timeoutStamp >> (i*2*8)) & 0x0000ffff;
        values.push_back(temp);
    }

    if (!modbusDevice)
        return;
    w->statusBar()->clearMessage();

    QModbusDataUnit writeUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, SENSORTS, 2);
    writeUnit.setValues(values);
    if (auto *reply = modbusDevice->sendWriteRequest(writeUnit, 1)) {
        if (!reply->isFinished()) {
            connect(reply, &QModbusReply::finished, this, [this, reply]() {
                MainWindow *mw = (MainWindow*) parentWidget();
                if (reply->error() == QModbusDevice::ProtocolError) {
                    mw->statusBar()->showMessage(tr("Write response error: %1 (Mobus exception: 0x%2)")
                        .arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16),
                        5000);
                } else if (reply->error() != QModbusDevice::NoError) {
                    mw->statusBar()->showMessage(tr("Write response error: %1 (code: 0x%2)").
                        arg(reply->errorString()).arg(reply->error(), -1, 16), 5000);
                }
                mw->statusBar()->showMessage(tr("OK!"));
                reply->deleteLater();
            });
        } else {
            // broadcast replies return immediately
            reply->deleteLater();
        }
    } else {
        w->statusBar()->showMessage(tr("Write error: ") + modbusDevice->errorString(), 5000);
    }
}

void systemDialog::handle_readready(QComboBox *cb)
{
    auto reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();
        cb->setCurrentIndex(unit.value(0));
    } else if (reply->error() == QModbusDevice::ProtocolError) {

    } else {

    }
    reply->deleteLater();
}

void systemDialog::handle_write(int addr, QComboBox *cb)
{
    MainWindow *w = (MainWindow*) parentWidget();
    QModbusClient* modbusDevice = w->getModbusDevice();
    if (!modbusDevice)
        return;

    quint16 value = cb->currentIndex();
    QModbusDataUnit writeUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, addr, 1);

    writeUnit.setValue(0, value);
    if (auto *reply = modbusDevice->sendWriteRequest(writeUnit, 1)) {
        if (!reply->isFinished()) {
            connect(reply, &QModbusReply::finished, this, [this, reply]() {
                MainWindow *w = (MainWindow*) parentWidget();
                if (reply->error() == QModbusDevice::ProtocolError) {
                   MainWindow *w = (MainWindow*) parentWidget();
                   w->statusBar()->showMessage(tr("Write response error: %1 (Mobus exception: 0x%2)")
                        .arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16),
                        5000);

                } else if (reply->error() != QModbusDevice::NoError) {
                   w->statusBar()->showMessage(tr("Write response error: %1 (code: 0x%2)").
                        arg(reply->errorString()).arg(reply->error(), -1, 16), 5000);
                } else {
                    w->statusBar()->showMessage(tr("OK!"));
                }

                reply->deleteLater();
            });
        } else {
            // broadcast replies return immediately
            reply->deleteLater();
        }
    } else {
        w->statusBar()->showMessage(tr("Write error: ") + modbusDevice->errorString(), 5000);
    }
}

void systemDialog::currentHoldReadReady()
{
    handle_readready(ui->currentHoldComboBox);
}

void systemDialog::on_currentHoldCheck_clicked()
{
    handle_read(CurrentHold, HoldEntries, &currentHoldReadReady);
}

void systemDialog::on_currentHoldSet_clicked()
{
    handle_write(CurrentHold, ui->currentHoldComboBox);
}

void systemDialog::voltageHoldReadReady()
{
    handle_readready(ui->voltageHoldComboBox);
}

void systemDialog::on_voltageHoldChk_clicked()
{
    handle_read(VoltageHold, HoldEntries, &voltageHoldReadReady);
}

void systemDialog::on_voltageHoldSet_clicked()
{
    handle_write(VoltageHold, ui->voltageHoldComboBox);
}

void systemDialog::PWM5ReadReady()
{
    handle_readready(ui->PWMComboBox);
}

void systemDialog::on_PWMChk_clicked()
{
    handle_read(PWM5, HoldEntries, &PWM5ReadReady);
}

void systemDialog::on_PWMSet_clicked()
{
    handle_write(PWM5, ui->PWMComboBox);
}

void systemDialog::PWMOCReadReady()
{
    handle_readready(ui->PWMOCComboBox);
}

void systemDialog::on_PWMOCChk_clicked()
{
    handle_read(PWMOC, HoldEntries, &PWMOCReadReady);
}

void systemDialog::on_PWMOCSet_clicked()
{
    handle_write(PWMOC, ui->PWMOCComboBox);
}
