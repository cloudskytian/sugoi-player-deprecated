#ifndef LOCATIONDIALOG_H
#define LOCATIONDIALOG_H

#include <QDialog>

namespace Ui {
class LocationDialog;
}

class LocationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LocationDialog(QString path, QWidget *parent = nullptr);
    ~LocationDialog() override;

    static QString getUrl(QString path, QWidget *parent = nullptr);

private slots:
    void validate(QString input);

private:
    Ui::LocationDialog *ui;
};

#endif // LOCATIONDIALOG_H
