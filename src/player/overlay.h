#pragma once

#include <QObject>

class QLabel;
class QImage;
class QTimer;

class Overlay : public QObject
{
    Q_OBJECT
public:
    explicit Overlay(QLabel *label, QImage *canvas, QTimer *timer, QObject *parent = nullptr);
    ~Overlay() override;

private:
    QLabel *label;
    QImage *canvas;
    QTimer *timer;
};
