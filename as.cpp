#include "autoscandialog.h"
#include "ui_autoscandialog.h"

autoScanDialog::autoScanDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::autoScanDialog)
{
    ui->setupUi(this);
}

autoScanDialog::~autoScanDialog()
{
    delete ui;
}
