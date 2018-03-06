#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

namespace Ui {
class AboutDialog;
}

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);
    ~AboutDialog();

    static void about(QWidget *parent = nullptr);

private:
    QString compilerText_HTML();
    QString compilerText_PlainText();
    QString aboutText_HTML();
    QString aboutText_PlainText();

private:
    Ui::AboutDialog *ui;
};

#endif // ABOUTDIALOG_H
