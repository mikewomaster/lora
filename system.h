#ifndef SYSTEM_H
#define SYSTEM_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>

QT_BEGIN_NAMESPACE

#define MASTER
#define VersionAddress (171 - 1)
#define VersionEntries 8
#define ResetEnableAddress (179 - 1)
#define ResetEntries 1
#define UsernameAddress (181 - 1)
#define UsernameEntries 8
#define PasswordAddress (189 - 1)
#define PasswordEntries 8

#define VoltageHold (307 - 1)
#define CurrentHold (308 - 1)
#define PWM5 (309 - 1)
#define PWMOC (310 - 1)
#define HoldEntries 1

namespace Ui {
class system;
}

QT_END_NAMESPACE

#define SNAddr2 (142 - 1)
#define SNEntries2 8

class systemDialog : public QDialog
{
    Q_OBJECT

public:
    explicit systemDialog(QWidget *parent = 0);
    ~systemDialog();

private slots:
    void on_systemCheck_clicked();
    void on_systemSet_clicked();
    void on_systemApply_clicked();
    void on_systemReload_clicked();
    void versionReadReady();
    void passwordReadReady();
    void usernameReadReady();
    void on_SNRead_clicked();
    void SNReadReady();
    void on_pushButton_clicked();
    void on_globalTSCheckPushButton_clicked();
    void on_globalTSSetPushButton_clicked();
    void globalTSChkReadReady();
    void PWM5ReadReady();
    void PWMOCReadReady();
    void currentHoldReadReady();
    void voltageHoldReadReady();
    void on_currentHoldCheck_clicked();
    void on_currentHoldSet_clicked();
    void on_voltageHoldChk_clicked();

    void on_voltageHoldSet_clicked();

    void on_PWMChk_clicked();

    void on_PWMSet_clicked();

    void on_PWMOCChk_clicked();

    void on_PWMOCSet_clicked();

private:
    Ui::system *ui;
    void handle_read(int addr, int entry, void (systemDialog::*fp)());
    void handle_readready(QLineEdit *le);
    void handle_readready(QComboBox *cb);
    void handle_write(int addr, int entry);
    void handle_write(int addr, QComboBox *cb);
    QString m_username;
    QString m_password;
};

#endif // SYSTEM_H
