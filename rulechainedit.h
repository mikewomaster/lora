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
    ~ruleChainEdit();

private:
    Ui::ruleChainEdit *ui;
};

#endif // RULECHAINEDIT_H
