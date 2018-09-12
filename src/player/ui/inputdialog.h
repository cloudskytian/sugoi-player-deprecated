#ifndef INPUTDIALOG_H
#define INPUTDIALOG_H

#include <QDialog>

#include <functional>

namespace Ui {
class InputDialog;
}

class InputDialog : public QDialog
{
    Q_OBJECT
public:
    explicit InputDialog(QString prompt, QString title, const std::function<bool (QString)> &validation, QWidget *parent = nullptr);
    ~InputDialog() override;

    static QString getInput(QString prompt, QString title, const std::function<bool (QString)> &validation, QWidget *parent = nullptr);

private slots:
    void validate(QString input);

private:
    Ui::InputDialog *ui;
    const std::function<bool (QString)> &validation;
};

#endif // INPUTDIALOG_H
