#ifndef OVERLAYHANDLER_H
#define OVERLAYHANDLER_H

#include <QObject>
#include <QHash>
#include <QMutex>
#include <QFont>
#include <QColor>
#include <QPoint>

class QTimer;

class SugoiEngine;
class Overlay;

class OverlayHandler : public QObject
{
    Q_OBJECT
public:
    explicit OverlayHandler(QObject *parent = nullptr);
    ~OverlayHandler() override;

public slots:
    void showStatusText(const QString &text, int duration = 4000);
    void showInfoText(bool show = true);
    void showText(const QString &text, QFont font, const QColor& color, QPoint pos, int duration, int id = -1);

protected slots:
    void remove(int id);

private:
    SugoiEngine *sugoi = nullptr;

    QHash<int, Overlay*> overlays;
    QMutex overlay_mutex;

    QTimer *refresh_timer = nullptr;
    int min_overlay,
        max_overlay,
        overlay_id;
};

#endif // OVERLAYHANDLER_H
