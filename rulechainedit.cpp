#include "rulechainedit.h"
#include "ui_rulechainedit.h"
#include "mainwindow.h"

#include <QMessageBox>

ruleChainEdit::ruleChainEdit(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ruleChainEdit)
{
    ui->setupUi(this);

    for (int i = 1; i <= 250; i++ ){
        ui->ruleChainInIDComboBox->addItem(QString::number(i));
        ui->ruleChainOutIDComBox->addItem(QString::number(i));
    }

    for (int i = 0; i < 8; i++){
        ui->ruleChainInChComboBox->addItem(QString::number(i));
        ui->ruleChainOutChComboBox->addItem(QString::number(i));
    }

    QStringList ruleType = {"V-V", "V-A", "A-V", "A-A"};
    ui->ruleChainRuleTypeComBox->addItems(ruleType);


    MainWindow *w = (MainWindow*) parentWidget();

    ui->ruleChainRuleNameEdit->text() = w->ruleChainList[term].ruleName;
    //ui->ruleChainInChComboBox->currentText().toShort() = w->ruleChainList[term].inDevCh;
    //ui->ruleChainInIDComboBox->currentText().toShort() = w->ruleChainList[term].inDevId;
    //ui->ruleChainOutChComboBox->currentText().toShort() = w->ruleChainList[term].outDevCh;
    //ui->ruleChainOutIDComBox->currentText().toShort() = w->ruleChainList[term].outDevId;
    //ui->ruleChainRuleTypeComBox->currentIndex() = w->ruleChainList[term].ruleType;
}

ruleChainEdit::ruleChainEdit(QWidget *parent, int t):
    QDialog(parent),
    ui(new Ui::ruleChainEdit),
    term(t)
{
    ui->setupUi(this);

    for (int i = 1; i <= 250; i++ ){
        ui->ruleChainInIDComboBox->addItem(QString::number(i));
        ui->ruleChainOutIDComBox->addItem(QString::number(i));
    }

    for (int i = 0; i < 8; i++){
        ui->ruleChainInChComboBox->addItem(QString::number(i));
        ui->ruleChainOutChComboBox->addItem(QString::number(i));
    }

    QStringList ruleType = {"V-V", "V-A", "A-V", "A-A"};
    ui->ruleChainRuleTypeComBox->addItems(ruleType);

    // set
    MainWindow *w = (MainWindow*) parentWidget();

    ui->ruleChainRuleNameEdit->setText(w->ruleChainList[term].ruleName);
    ui->ruleChainInChComboBox->setCurrentIndex(w->ruleChainList[term].inDevCh);
    ui->ruleChainInIDComboBox->setCurrentIndex(w->ruleChainList[term].inDevId - 1);
    ui->ruleChainOutChComboBox->setCurrentIndex(w->ruleChainList[term].outDevCh);
    ui->ruleChainOutIDComBox->setCurrentIndex(w->ruleChainList[term].outDevId - 1);
    ui->ruleChainRuleTypeComBox->setCurrentIndex(w->ruleChainList[term].ruleType);
}

ruleChainEdit::~ruleChainEdit()
{
    delete ui;
}

void ruleChainEdit::on_ruleChainEditPushButton_clicked()
{

    MainWindow *w = (MainWindow*) parentWidget();
    if((ui->ruleChainRuleNameEdit->text().length() > 8) || (ui->ruleChainRuleNameEdit->text().length() == 0)){
        QMessageBox::information(NULL, "Error", "Please set Rule-Name with maximum 8 words.");
        return;
    }

    w->ruleChainList[term].ruleName = ui->ruleChainRuleNameEdit->text();
    w->ruleChainList[term].inDevCh = ui->ruleChainInChComboBox->currentText().toShort();
    w->ruleChainList[term].inDevId = ui->ruleChainInIDComboBox->currentText().toShort();
    w->ruleChainList[term].outDevCh = ui->ruleChainOutChComboBox->currentText().toShort();
    w->ruleChainList[term].outDevId = ui->ruleChainOutIDComBox->currentText().toShort();
    w->ruleChainList[term].ruleType = ui->ruleChainRuleTypeComBox->currentIndex();

    emit(ruleChainSignal(term));
    close();
}
