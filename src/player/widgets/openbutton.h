#pragma once

#include <QPushButton>

class OpenButton : public QPushButton
{
    Q_OBJECT
public:
    explicit OpenButton(QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *event) override;

signals:
    void LeftClick();
    void MiddleClick();
    void RightClick();
};
