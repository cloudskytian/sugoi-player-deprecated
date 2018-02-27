#include "progressindicatorbar.h"

#include <QPainter>
#include <QMouseEvent>

ProgressIndicatorBar::ProgressIndicatorBar(QWidget *parent) : QWidget(parent)
  , m_currentProgress(0)
  , min(0)
  , max(1000)
{
    setWindowFlags(Qt::FramelessWindowHint/* | Qt::WindowStaysOnTopHint*/);
}

void ProgressIndicatorBar::setMinimum(int value)
{
    min = value;
    emit rangeChanged(min, max);
}

int ProgressIndicatorBar::minimum() const
{
    return min;
}

void ProgressIndicatorBar::setMaximum(int value)
{
    max = value;
    emit rangeChanged(min, max);
}

int ProgressIndicatorBar::maximum() const
{
    return max;
}

void ProgressIndicatorBar::setRange(int min_value, int max_value)
{
    if (min_value >= max_value)
    {
        return;
    }
    min = min_value;
    max = max_value;
    emit rangeChanged(min, max);
}

void ProgressIndicatorBar::setValue(int value)
{
    if (value <= min)
    {
        m_currentProgress = min;
    }
    else if (value >= max)
    {
        m_currentProgress = max;
    }
    else
    {
        m_currentProgress = value;
    }
    update();
    emit sliderMoved(value);
}

int ProgressIndicatorBar::value() const
{
    return m_currentProgress;
}

void ProgressIndicatorBar::paintEvent(QPaintEvent *event)
{
    QPainter objPainter(this);
    objPainter.setRenderHint(QPainter::Antialiasing);
    //绘背景
    objPainter.fillRect(rect(), Qt::black);
    //绘内容区背景
    objPainter.fillRect(contentsRect(), Qt::gray);
    int range = max - min;
    if (range <= 1)
    {
        range = 1;
    }
    int nWidth = static_cast<float>(contentsRect().width())
            * static_cast<float>(m_currentProgress) / static_cast<float>(range);
    //绘进度条背景;
    objPainter.fillRect(contentsRect().x(),contentsRect().y(),nWidth,contentsRect().height(), Qt::red);

    QWidget::paintEvent(event);
}

void ProgressIndicatorBar::mousePressEvent(QMouseEvent *event)
{
    int range = max - min;
    if (range <= 1)
    {
        range = 1;
    }
    int value = (static_cast<float>(event->pos().x()) / static_cast<float>(width()))
            * static_cast<float>(range);
    setValue(value);
    QWidget::mousePressEvent(event);
}
