#ifndef asDialog_H
#define asDialog_H

#include <QDialog>
#include "mapmodel.h"

#define MapEnableAddress (811 - 1)
#define MapEnableUnits 3

#define MapSetAddress (814 - 1)
#define MapSetUnits 20

namespace Ui {
class asDialog;
}

class asDialog : public QDialog
{
    Q_OBJECT

public:
    explicit asDialog(QWidget *parent = 0);
    ~asDialog();
    mapModel *m_Model;
    void init();

private:
    Ui::asDialog *ui;
    void dullSecondTimeApply();


private slots:
    void on_checkMapPushButton_clicked();
    void on_applyMapPushButton_clicked();
    void on_asCheckPushButton_clicked();
    void on_appPushButton_clicked();

    void mapEnableSlot();
    void mapSetSlot();
};

#endif // asDialog_H
