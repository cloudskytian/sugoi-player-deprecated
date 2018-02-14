#ifndef LOGOWIDGET_H
#define LOGOWIDGET_H

#include <QWidget>
#include <QImage>

class LogoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LogoWidget(QWidget *parent = nullptr, const QString &imageFilePath = QString::fromLatin1(":/images/logo.png"));

signals:

public slots:
    void SetLogoImage(const QString &imageFilePath);

protected:
    virtual void paintEvent(QPaintEvent *event);

private:
    QImage m_logoImage;
};

#endif // LOGOWIDGET_H
