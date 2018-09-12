#ifndef CUSTOMSLIDER_H
#define CUSTOMSLIDER_H

#include <QSlider>

class CustomSlider : public QSlider
{
    Q_OBJECT
public:
    explicit CustomSlider(QWidget *parent = nullptr);

public slots:
    void setValueNoSignal(int value);

protected:
    void mousePressEvent(QMouseEvent *event) override;
};

#endif // CUSTOMSLIDER_H
