#ifndef AUTOSCANDIALOG_H
#define AUTOSCANDIALOG_H

#include <QDialog>

namespace Ui {
class autoScanDialog;
}

class autoScanDialog : public QDialog
{
    Q_OBJECT

public:
    explicit autoScanDialog(QWidget *parent = 0);
    ~autoScanDialog();

private:
    Ui::autoScanDialog *ui;
};

#endif // AUTOSCANDIALOG_H
