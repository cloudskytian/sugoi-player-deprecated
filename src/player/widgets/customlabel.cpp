#include "customlabel.h"

#include <QMouseEvent>

CustomLabel::CustomLabel(QWidget *parent) :
    QLabel(parent)
{
}

void CustomLabel::mousePressEvent(QMouseEvent *event)
{
    if(isEnabled() && event->button() == Qt::LeftButton)
    {
        emit clicked();
        event->accept();
    }
    else {
        QLabel::mousePressEvent(event);
    }
}
