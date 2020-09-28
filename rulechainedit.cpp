#include "rulechainedit.h"
#include "ui_rulechainedit.h"

ruleChainEdit::ruleChainEdit(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ruleChainEdit)
{
    ui->setupUi(this);
}

ruleChainEdit::~ruleChainEdit()
{
    delete ui;
}
