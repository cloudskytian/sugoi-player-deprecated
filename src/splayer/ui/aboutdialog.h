#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class AboutDialog;
}

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QString lang = QString::fromLatin1("en"), QWidget *parent = 0);
    ~AboutDialog();

    static void about(QString lang = QString::fromLatin1("en"), QWidget *parent = 0);

private:
    QString aboutText_HTML();
    QString aboutText_PlainText();
    QString creditsText_HTML();
    QString creditsText_PlainText();
    QString licenseText_HTML(QString lang = QString::fromLatin1("en"));
    QString licenseText_PlainText();

private:
    Ui::AboutDialog *ui;
};

#endif // ABOUTDIALOG_H
