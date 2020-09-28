#ifndef RULECHAINEDIT_H
#define RULECHAINEDIT_H

#include <QDialog>

namespace Ui {
    class ruleChainEdit;
}

class ruleChainEdit : public QDialog
{
    Q_OBJECT

public:
    explicit ruleChainEdit(QWidget *parent = 0);
    ruleChainEdit(QWidget *parent = 0, int t = 0);
    ~ruleChainEdit();
    int term;

Q_SIGNALS:
    void ruleChainSignal(int);

private slots:
    void on_ruleChainEditPushButton_clicked();

private:
    Ui::ruleChainEdit *ui;
};

#endif // RULECHAINEDIT_H
