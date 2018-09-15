#pragma once

#include <QDialog>

namespace Ui {
class JumpDialog;
}

class JumpDialog : public QDialog
{
    Q_OBJECT

public:
    explicit JumpDialog(int maxTime, QWidget *parent = nullptr);
    ~JumpDialog() override;

    static int getTime(int maxTime, QWidget *parent = nullptr);

private slots:
    void validate();

private:
    Ui::JumpDialog *ui;

    int time, maxTime;
};
