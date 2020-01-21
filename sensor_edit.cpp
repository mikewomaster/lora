#include "sensor_edit.h"
#include "ui_sensor_edit.h"
#include "mainwindow.h"
#include "sensor.h"

sensor_edit::sensor_edit(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::sensor_edit)
{
    ui->setupUi(this);
}

sensor_edit::~sensor_edit()
{
    delete ui;
}

void sensor_edit::on_sensorAddPushButton_clicked()
{
     MainWindow *w = (MainWindow*) parentWidget();
     w->sensorRecordList[seq].type = ui->sensorTypeCombox->currentText();
     w->sensorRecordList[seq].type_ = ui->sensorTypeCombox->currentIndex() + 1;
     w->sensorRecordList[seq].id = ui->sensorSlaveId->text().toShort();
     w->sensorRecordList[seq].reg_addr = ui->sensorPLCAddress->text().toInt();
     w->sensorRecordList[seq].value = "null";
     w->sensor_edit_flag = true;
     close();
}
