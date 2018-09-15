#pragma once

#include "customslider.h"

class SeekBar : public CustomSlider
{
    Q_OBJECT
public:
    explicit SeekBar(QWidget *parent = nullptr);

public slots:
    void setTracking(int _totalTime);
    void setTicks(QList<int> values);

protected:
    QString formatTrackingTime(int _time);

    void mouseMoveEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    QList<int> ticks;
    bool tickReady;
    int totalTime;
};
