#ifndef PROGRESSINDICATORBAR_H
#define PROGRESSINDICATORBAR_H

#include <QWidget>

class ProgressIndicatorBar : public QWidget
{
    Q_OBJECT

public:
    explicit ProgressIndicatorBar(QWidget *parent = nullptr);

signals:
    void sliderMoved(int value);
    void rangeChanged(int min_value, int max_value);

public slots:
    void setMinimum(int value = 0);
    int minimum() const;
    void setMaximum(int value = 1000);
    int maximum() const;
    void setRange(int min_value = 0, int max_value = 1000);
    void setValue(int value = 0);
    int value() const;

protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);

private:
    int m_currentProgress, min, max;
};

#endif // PROGRESSINDICATORBAR_H
