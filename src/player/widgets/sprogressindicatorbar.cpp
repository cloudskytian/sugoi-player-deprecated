#include "sprogressindicatorbar.h"

#include <QPainter>
#include <QMouseEvent>

SProgressIndicatorBar::SProgressIndicatorBar(QWidget *parent) : QWidget(parent)
  , m_currentProgress(0)
  , min(0)
  , max(1000)
{
    setWindowFlags(Qt::FramelessWindowHint/* | Qt::WindowStaysOnTopHint*/);
}

void SProgressIndicatorBar::setMinimum(int value)
{
    min = value;
    emit rangeChanged(min, max);
}

int SProgressIndicatorBar::minimum() const
{
    return min;
}

void SProgressIndicatorBar::setMaximum(int value)
{
    max = value;
    emit rangeChanged(min, max);
}

int SProgressIndicatorBar::maximum() const
{
    return max;
}

void SProgressIndicatorBar::setRange(int min_value, int max_value)
{
    if (min_value >= max_value)
    {
        return;
    }
    min = min_value;
    max = max_value;
    emit rangeChanged(min, max);
}

void SProgressIndicatorBar::setValue(int value)
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

int SProgressIndicatorBar::value() const
{
    return m_currentProgress;
}

void SProgressIndicatorBar::paintEvent(QPaintEvent *event)
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

void SProgressIndicatorBar::mousePressEvent(QMouseEvent *event)
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
