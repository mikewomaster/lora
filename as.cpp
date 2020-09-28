#include <stdlib.h>

#include <QModbusDataUnit>
#include <QModbusRtuSerialMaster>
#include <QDebug>
#include <QMessageBox>

#include "asdialog.h"
#include "ui_asDialog.h"
#include "mapmodel.h"
#include "mainwindow.h"

asDialog::asDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::asDialog),
    m_Model(new mapModel())
{
   ui->setupUi(this);
   init();
}

asDialog::~asDialog()
{
    delete ui;
}

void asDialog::init()
{
    setFixedWidth(348);
    setFixedHeight(564);

    ui->mapTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->mapTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->mapTableView->setShowGrid(false);
    ui->mapTableView->setFrameShape(QFrame::NoFrame);
    ui->mapTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->mapTableView->setModel(m_Model);
    ui->mapTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->pushButton->setVisible(false); // to cancel 360 warning without reason

    QList<mapModelData> recordList;
    for (int i = 1; i <= 20; ++i)
    {
     mapModelData record;
     record.id = 0;
     record.reg = 0;
     recordList.append(record);
    }

    m_Model->updateData(recordList);
}

void asDialog::mapEnableSlot()
{
    auto reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();
        unit.value(0) == 0 ? ui->mapEnableRadioButton->setChecked(false) : ui->mapEnableRadioButton->setChecked(true);
        ui->mapLoopLineEdit->setText(QString::number(unit.value(1)));
        ui->mapIntervalLineEdit->setText(QString::number(unit.value(2)));
    } else if (reply->error() == QModbusDevice::ProtocolError) {

    } else {

    }
    reply->deleteLater();
}

void asDialog::on_checkMapPushButton_clicked()
{
    MainWindow *w = (MainWindow*) parentWidget();
    QModbusClient* modbusDevice = w->getModbusDevice();

    if (!modbusDevice)
        return;

    QModbusDataUnit readUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, MapEnableAddress, MapEnableUnits);

    if (auto *reply = modbusDevice->sendReadRequest(readUnit, 1)) {
        if (!reply->isFinished())
            connect(reply, &QModbusReply::finished, this, &asDialog::mapEnableSlot);
        else
            delete reply;
    } else {

    }
}

void asDialog::on_applyMapPushButton_clicked()
{
    MainWindow *w = (MainWindow*) parentWidget();
    QModbusClient* modbusDevice = w->getModbusDevice();

    if (!modbusDevice)
        return;

    QModbusDataUnit writeUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, MapEnableAddress, MapEnableUnits);
    quint16 enableValue;
    ui->mapEnableRadioButton->isChecked() ? enableValue = 1 : enableValue = 0;
    writeUnit.setValue(0, enableValue);
    writeUnit.setValue(1, ui->mapLoopLineEdit->text().toInt());
    writeUnit.setValue(2, ui->mapIntervalLineEdit->text().toInt());

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
            reply->deleteLater();
        }
    } else {
        w->statusBar()->showMessage(tr("Write error: ") + modbusDevice->errorString(), 5000);
    }
}

void asDialog::mapSetSlot()
{
    auto reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();
        QList<mapModelData> recordList;
        mapModelData tempData;

        for (int i = 0; i < unit.valueCount(); i += 2) {
           tempData.id = unit.value(i);
           tempData.reg = unit.value(i + 1);
           recordList.append(tempData);
        }

        tempData.id = 0;
        tempData.reg = 0;
        for (int i = recordList.size(); i < 20; i++)
            recordList.append(tempData);

        m_Model->updateData(recordList);
    } else if (reply->error() == QModbusDevice::ProtocolError) {

    } else {

    }
    reply->deleteLater();

}

void asDialog::on_asCheckPushButton_clicked()
{
    MainWindow *w = (MainWindow*) parentWidget();
    QModbusClient* modbusDevice = w->getModbusDevice();

    if (!modbusDevice)
        return;

    QModbusDataUnit readUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, MapSetAddress, MapSetUnits+MapSetUnits);

    if (auto *reply = modbusDevice->sendReadRequest(readUnit, 1)) {
        if (!reply->isFinished())
            connect(reply, &QModbusReply::finished, this, &asDialog::mapSetSlot);
        else
            delete reply;
    } else {

    }
}

void asDialog::dullSecondTimeApply()
{
    MainWindow *w = (MainWindow*) parentWidget();
    QModbusClient* modbusDevice = w->getModbusDevice();

    if (!modbusDevice)
        return;

    QModbusDataUnit writeUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, MapSetAddress + MapSetUnits, MapSetUnits);

    QVector<quint16> values;
    for (int i = 10; i < 20; i++) {
        quint16 unit1 = m_Model->data(m_Model->index(i, 0)).toInt();
        quint16 unit2 = m_Model->data(m_Model->index(i, 1)).toInt();

        if (unit1 == 0 && unit2 == 0)
            continue;
        else {
            values.push_back(unit1);
            values.push_back(unit2);
        }
    }
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
            reply->deleteLater();
        }
    } else {
        w->statusBar()->showMessage(tr("Write error: ") + modbusDevice->errorString(), 5000);
    }
}

void asDialog::on_appPushButton_clicked()
{
    MainWindow *w = (MainWindow*) parentWidget();
    QModbusClient* modbusDevice = w->getModbusDevice();

    if (!modbusDevice)
        return;

    QModbusDataUnit writeUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, MapSetAddress, MapSetUnits);

    QVector<quint16> values;
    for (int i = 0; i < 10; i++) {
        quint16 unit1 = m_Model->data(m_Model->index(i, 0)).toInt();
        quint16 unit2 = m_Model->data(m_Model->index(i, 1)).toInt();

        if (unit1 == 0 && unit2 == 0)
            continue;
        else {
            values.push_back(unit1);
            values.push_back(unit2);
        }
    }
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
            reply->deleteLater();
        }
    } else {
        w->statusBar()->showMessage(tr("Write error: ") + modbusDevice->errorString(), 5000);
    }

    _sleep(1800);
    dullSecondTimeApply();
}
