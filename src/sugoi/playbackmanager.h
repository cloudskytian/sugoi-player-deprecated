#ifndef PLAYBACKMANAGER_H
#define PLAYBACKMANAGER_H

// A simple playback manager designed to control the MainWindow like it were a
// widget.  Due to needing to communicate with the playlists, it sort of takes
// control of the mpv widget away from its host.

#include <QObject>

class MpvObject;
class MainWindow;
class PropertiesWindow;

class PlaybackManager : public QObject
{
    Q_OBJECT
public:
    explicit PlaybackManager(QObject *parent = nullptr);
    ~PlaybackManager();

signals:

public slots:
    MpvObject *mpvObject() const;
    MainWindow *mainWindow() const;

private:
    MpvObject *m_pMpvObject = nullptr;
    MainWindow *m_pMainWindow = nullptr;
    PropertiesWindow *m_pPropertiesWindow = nullptr;
};

#endif // PLAYBACKMANAGER_H
