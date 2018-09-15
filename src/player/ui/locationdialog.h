#pragma once

#include <QDialog>

namespace Ui {
class LocationDialog;
}

class LocationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LocationDialog(const QString& path, QWidget *parent = nullptr);
    ~LocationDialog() override;

    static QString getUrl(const QString& path, QWidget *parent = nullptr);

private slots:
    void validate(const QString& input);

private:
    Ui::LocationDialog *ui;
};
