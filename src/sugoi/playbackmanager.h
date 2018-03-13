#ifndef PLAYBACKMANAGER_H
#define PLAYBACKMANAGER_H

// A simple playback manager designed to control the MainWindow like it were a
// widget.  Due to needing to communicate with the playlists, it sort of takes
// control of the mpv widget away from its host.

#include <QObject>

#include "fileassoc.h"

class MpvObject;
class MainWindow;
class PropertiesWindow;

class PlaybackManager : public QObject
{
    Q_OBJECT
public:
    static PlaybackManager *instance();

private:
    explicit PlaybackManager(QObject *parent = nullptr);
    ~PlaybackManager();

signals:

public slots:
    void initMainWindow(bool backgroundMode = false);
    void showMainWindow();
    void hideMainWindow();
    void setBackgroundMode(bool enable = false);
    void closeMainWindow();

public slots:
    void load(const QString &path);

public slots:
    MpvObject *mpvObject() const;
    MainWindow *mainWindow() const;

private:
    MpvObject *m_pMpvObject = nullptr;
    MainWindow *m_pMainWindow = nullptr;
    PropertiesWindow *m_pPropertiesWindow = nullptr;

    bool currentShowFullscreenIndicator = true;
    FileAssoc::reg_type  currentRegType = FileAssoc::reg_type::ALL;
    FileAssoc::reg_state currentRegState = FileAssoc::reg_state::ALL_REGISTERED;
    bool currentAlwaysCheckFileAssoc = true;
    bool currentPauseWhenMinimized = true;
    bool currentOsdShowLocalTime = true;
    bool currentShowVideoPreview = true;
    bool currentAllowRunInBackground = true;
    bool currentQuickStartMode = true;
    bool currentTrayIconVisible = true;
    bool currentAutoUpdatePlayer = true;
    bool currentAutoUpdateStreamingSupport = true;
    QString currentSkinFile;
    bool currentPlayInBackground = false;
    QString currentUiLanguage = QLatin1String("auto");
    QString currentOnTop;
    int currentAutoFit = 100;
    int currentMaxRecent = 100;
    bool currentHidePopup = false;
    bool currentRemaining = false;
    bool currentScreenshotDialog = false;
    bool currentDebug = false;
    bool currentResume = false;
    bool currentHideAllControls = false;
};

#endif // PLAYBACKMANAGER_H
