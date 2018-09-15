#pragma once

#include <QLineEdit>

class CustomLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit CustomLineEdit(QWidget *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *event) override;

signals:
    void submitted(QString);
};
