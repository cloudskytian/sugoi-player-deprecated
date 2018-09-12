#ifndef SYSINFODIALOG_H
#define SYSINFODIALOG_H

#include <QDialog>

namespace Ui {
class SysInfoDialog;
}

class SysInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SysInfoDialog(QWidget *parent = nullptr);
    ~SysInfoDialog() override;

private slots:
    QString sysInfoText_HTML();
    QString sysInfoText_PlainText();

private:
    Ui::SysInfoDialog *ui;
};

#endif // SYSINFODIALOG_H
