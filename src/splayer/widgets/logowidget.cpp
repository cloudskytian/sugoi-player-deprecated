#include "logowidget.h"

#include <QSizePolicy>
#include <QPainter>

LogoWidget::LogoWidget(QWidget *parent, const QString &imageFilePath) : QWidget(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    SetLogoImage(imageFilePath);
}

void LogoWidget::SetLogoImage(const QString &imageFilePath)
{
    m_logoImage = QImage(imageFilePath);
    update();
}

void LogoWidget::paintEvent(QPaintEvent *event)
{
    if (!m_logoImage.isNull())
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.drawImage(geometry(), m_logoImage.scaled(size()));
    }
    QWidget::paintEvent(event);
}
