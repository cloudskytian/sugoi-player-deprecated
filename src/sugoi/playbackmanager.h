#ifndef PLAYBACKMANAGER_H
#define PLAYBACKMANAGER_H

#include <QObject>

class PlaybackManager : public QObject
{
    Q_OBJECT
public:
    explicit PlaybackManager(QObject *parent = nullptr);

signals:

public slots:
};

#endif // PLAYBACKMANAGER_H