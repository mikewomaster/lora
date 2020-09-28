#include <QMessageBox>
#include <QModbusDataUnit>
#include <QModbusTcpClient>
#include <QModbusRtuSerialMaster>

#include "mainwindow.h"
#include "rulechainmodbus.h"
#include "asdialog.h"

void MainWindow::chainRuleTableReadReady()
{
    handle_read_ready(ui->ruleIntervalLineEdit);
}

void MainWindow::on_ruleIntervalChkPushButton_clicked()
{
    handle_read(ruleChainTable, &chainRuleTableReadReady);
}

void MainWindow::on_ruleIntervalSetPushButton_clicked()
{
    handle_write(ui->ruleIntervalLineEdit, ruleChainTable);
}

/*
 * rule chain table
 * clear table
 * add / delete / modify / check
*/
void MainWindow::on_ruleClrPushButton_clicked()
{
    for (int i =0; i < 40; i++) {
        ruleChainList[i].ruleName = "";
        ruleChainList[i].inDevCh = 0;
        ruleChainList[i].inDevId = 0;
        ruleChainList[i].ruleType = 0;
        ruleChainList[i].outDevId = 0;
        ruleChainList[i].outDevCh = 0;
        ui->RuleVIew->setRowHidden(i, true);
    }
}

void MainWindow::on_ruleChainAddPushButton_clicked()
{
    statusBar()->clearMessage();

    rule_chain unit;
    unit.ruleName = ui->ruleNameLineEdit->text();
    if (unit.ruleName.length() > 8) {
        QMessageBox::information(NULL, "Error", "Please set Rule-Name with maximum 8 words.");
        return;
    } else if (unit.ruleName.length() == 0) {
        QMessageBox::information(NULL, "Error", "Please set Rule Name.");
        return;
    }

    unit.inDevId = ui->ruleInDeviceIDComboBox->currentText().toShort();
    unit.inDevCh = ui->ruleInChComboBox->currentText().toShort();

    unit.outDevId = ui->ruleOutIDComboBox->currentText().toShort();
    unit.outDevCh = ui->ruleOutChnComboBox->currentText().toShort();

    unit.ruleType = ui->ruleTypeComboBox->currentIndex();

    int i = 0;
    for (i = 0; i < 50; i++){
        if (ruleChainList[i].ruleName == ""){
            ruleChainAddModbus(unit, i);
            unit.seq = i;
            ruleChainList[i] = unit;
            break;
        }
    }

    m_ruleChainModel->updateData(ruleChainList);
    ui->RuleVIew->setRowHidden(i, false);
}

void MainWindow::ruleChainAddModbus(rule_chain unit, int term)
{
    if (!modbusDevice)
        return;

    QModbusDataUnit writeUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, ruleChainUnit + (term*ruleChainUnitLength), ruleChainUnitLength);
    QVector<quint16> values;

    int i = 0;
    for (i = 0; i < unit.ruleName.size(); i++) {
        if ((i+1) % 2 == 0) {
            quint16 temp = unit.ruleName.at(i - 1).toLatin1();
            temp = (temp << 8) + unit.ruleName.at(i).toLatin1();
            values.push_back(temp);
        }
    }

    if (i % 2) {
        quint16 temp = unit.ruleName.at(i-1).toLatin1();
        temp = temp << 8;
        values.push_back(temp);
        i++;
    }

    for (i = (i / 2); i < 4; i++) {
        values.push_back(0x0000);
    }

    values.push_back(unit.inDevId);
    values.push_back(unit.inDevCh);
    values.push_back(unit.outDevId);
    values.push_back(unit.outDevCh);
    values.push_back(unit.ruleType);
    writeUnit.setValues(values);

    handle_write(writeUnit);
}

void MainWindow::ruleChainDelete()
{
    if (!modbusDevice)
        return;

    int row = ui->RuleVIew->currentIndex().row();
    QAbstractItemModel *model = ui->RuleVIew->model();
    QModelIndex index = model->index(row, 6);
    QVariant data = model->data(index);
    int term = data.toInt();

    ruleChainList[term].ruleName = "";
    ruleChainList[term].inDevCh = 0;
    ruleChainList[term].inDevCh = 0;
    ruleChainList[term].outDevId = 0;
    ruleChainList[term].outDevCh = 0;
    ruleChainList[term].ruleType = 0;

    QModbusDataUnit writeUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, ruleChainUnit + term*ruleChainUnitLength, ruleChainUnitLength);

    for (int i = 0; i < ruleChainUnitLength; i++)
        writeUnit.setValue(i, 0);

    handle_write(writeUnit);

    ui->RuleVIew->setRowHidden(term, true);
}

void MainWindow::ruleChainEditSlot(int term)
{
    rule_chain unit = ruleChainList[term];
    ruleChainAddModbus(unit, term);
    m_ruleChainModel->updateData(ruleChainList);
}

void MainWindow::ruleChainEditAction()
{
    int row = ui->RuleVIew->currentIndex().row();
    QAbstractItemModel *model = ui->RuleVIew->model();
    QModelIndex index = model->index(row, 6);
    QVariant data = model->data(index);
    int term = data.toInt();

    m_ruleChainEdit = new ruleChainEdit(this, term);
    m_ruleChainEdit->setWindowTitle("Rule Chain Configuration");
    // m_ruleChainEdit->term = term;

    connect(m_ruleChainEdit, SIGNAL(ruleChainSignal(int)), this, SLOT(ruleChainEditSlot(int)));
    m_ruleChainEdit->show();
}


/*
 * #define ruleChainUnitLength (9)
    #define totalTable 40
    #define ruleChainTableTimes 4
    #define ruleChainTableEachTimeUnit 90
    #define ruleChainUnitEachTime 10
*/
static int __ruleChainTime;
void MainWindow::ruleChainTableReadReady()
{
    auto reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

    if (reply->error() == QModbusDevice::NoError) {
        m_ruleChainModel->clearDate(ruleChainList);
        const QModbusDataUnit unit = reply->result();

        for (int i = 0; i < ruleChainUnitEachTime; i++) {
            QString s = "";
            int stringNumber = 0;
            for (int j = ruleChainUnitLength*i + 0; j < ruleChainUnitLength*i + 4; j++) {
                if ((unit.value(j) >> 8) == 0x00)
                    break;
                s[2*stringNumber] = unit.value(j) >> 8;
                if ((unit.value(j) & 0x00ff) == 0x00)
                    break;
                s[(2*stringNumber) +1] = unit.value(j) & 0x00ff;
                stringNumber ++;
            }
            ruleChainList[(ruleChainUnitEachTime*__ruleChainTime)+i].ruleName = s;
            ruleChainList[(ruleChainUnitEachTime*__ruleChainTime)+i].inDevId = unit.value(ruleChainUnitLength*i + 4);
            ruleChainList[(ruleChainUnitEachTime*__ruleChainTime)+i].inDevCh = unit.value(ruleChainUnitLength*i + 5);
            ruleChainList[(ruleChainUnitEachTime*__ruleChainTime)+i].outDevId = unit.value(ruleChainUnitLength*i + 6);
            ruleChainList[(ruleChainUnitEachTime*__ruleChainTime)+i].outDevCh = unit.value(ruleChainUnitLength*i + 7);
            ruleChainList[(ruleChainUnitEachTime*__ruleChainTime)+i].ruleType = unit.value(ruleChainUnitLength*i + 8);
        }
    }else if (reply->error() == QModbusDevice::ProtocolError) {
            statusBar()->showMessage(tr("Read response error: %1 (Mobus exception: 0x%2)").
                                        arg(reply->errorString()).
                                        arg(reply->rawResult().exceptionCode(), -1, 16), 5000);
    } else {
        statusBar()->showMessage(tr("Read response error: %1 (code: 0x%2)").
                                    arg(reply->errorString()).
                                    arg(reply->error(), -1, 16), 5000);
    }
    (__ruleChainTime == 3) ? (__ruleChainTime = 0) : (__ruleChainTime++);
    reply->deleteLater();
}

void MainWindow::on_ruleChkPushButton_clicked()
{
    if (!modbusDevice)
        return;
    statusBar()->clearMessage();
    ui->ruleChkPushButton->setEnabled(false);

    for (int i = 0; i < ruleChainTableTimes; i++) {
        quint16 ADDR =  ruleChainUnit + (ruleChainTableEachTimeUnit * i);
        QModbusDataUnit readUnit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters, ADDR, ruleChainTableEachTimeUnit);

        if (auto *reply = modbusDevice->sendReadRequest(readUnit, ui->serverEdit->value())) {
            if (!reply->isFinished()) {
                connect(reply, &QModbusReply::finished, this, &MainWindow::ruleChainTableReadReady);
            }
            else
                delete reply;
        } else {
            statusBar()->showMessage(tr("Read error: ") + modbusDevice->errorString(), 5000);
        }
        _sleep(1000);
    }
    _sleep(6000);

    QMessageBox::information(nullptr, "Read Value", "Read rule chain table configuration done, thanks for patience.");

    for (int i = 0; i < totalTable; i++) {
         if (ruleChainList[i].ruleName != "")
            ui->RuleVIew->setRowHidden(i, false);
    }

    m_ruleChainModel->updateData(ruleChainList);
    ui->ruleChkPushButton->setEnabled(true);
}
