﻿#pragma once

#include <utility>

#include <QTimer>
#include <QHash>
#include <QAction>

#include "ui/cframelesswindow.h"
#include "recent.h"
#ifdef Q_OS_WIN
#include "fileassoc.h"
#endif
#include "widgets/progressindicatorbar.h"

#if defined(Q_OS_WIN) && defined(QT_HAS_WINEXTRAS)
class QWinThumbnailToolBar;
class QWinThumbnailToolButton;
class QWinTaskbarButton;
class QWinTaskbarProgress;
class QWinJumpList;
#endif

class SugoiEngine;
class MpvWidget;

namespace Ui {
class MainWindow;
}

class MainWindow : public CFramelessWindow
{
friend class SugoiEngine;
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    QString getLang()          { return lang; }
    QString getOnTop()         { return onTop; }
    int getAutoFit()           { return autoFit; }
    int getMaxRecent()         { return maxRecent; }
    bool getHidePopup()        { return hidePopup; }
    bool getRemaining()        { return remaining; }
    bool getScreenshotDialog() { return screenshotDialog; }
    bool getDebug()            { return debug; }
    bool getResume()           { return resume; }
    bool getHideAllControls()  { return hideAllControls; }
    bool isFullScreenMode()    { return hideAllControls || isFullScreen(); }
#ifdef Q_OS_WIN
    FileAssoc::reg_type getFileAssocType() { return regType; }
    FileAssoc::reg_state getFileAssocState() { return regState; }
    bool getAlwaysCheckFileAssoc() { return alwaysCheckFileAssoc; }
#endif
    bool getShowFullscreenIndicator() { return showFullscreenIndicator; }
    bool getOSDShowLocalTime()   { return osdShowLocalTime; }
    bool getPauseWhenMinimized() { return pauseWhenMinimized; }
    bool getAllowRunInBackground() { return allowRunInBackground; }
    QString getSkinFile()        { return skinFile; }

    Ui::MainWindow *ui = nullptr;
    QImage albumArt;

public slots:
    void MapShortcuts();
#ifdef Q_OS_WIN
    void SetFileAssoc(FileAssoc::reg_type type = FileAssoc::reg_type::ALL, bool showUI = false);
#endif
    void initMainWindow();
    void bringToFront();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;    // drag file into
    void dropEvent(QDropEvent *event) override;              // drop file into
    void mousePressEvent(QMouseEvent *event) override;       // pressed mouse down
    void mouseMoveEvent(QMouseEvent *event) override;        // moved mouse on the form
    void mouseDoubleClickEvent(QMouseEvent *event) override; // double clicked the form
    bool eventFilter(QObject *obj, QEvent *event) override;  // event filter (get mouse move events from mpvFrame)
    void wheelEvent(QWheelEvent *event) override;            // the mouse wheel is used
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void changeEvent(QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void HideAllControls(bool h, bool s = true);    // hideAllControls--s option is used by fullscreen
    void FullScreen(bool fs);                       // makes window fullscreen
    void ShowPlaylist(bool visible);                // sets the playlist visibility
    void HideAlbumArt(bool hide);                   // hides the album art
    void UpdateRecentFiles();                       // populate recentFiles menu
    void SetPlayButtonIcon(bool play);
    void SetNextButtonEnabled(bool enable);
    void SetPreviousButtonEnabled(bool enable);
    void SetRemainingLabels(int time);
    bool IsPlayingMusic(const QString &filePath);
    bool IsPlayingVideo(const QString &filePath);
    void connectMpvSignalsAndSlots();
    void disconnectMpvSignalsAndSlots();
    void reconnectMpvSignalsAndSlots();
    void connectUiSignalsAndSlots();
    void disconnectUiSignalsAndSlots();
    void reconnectUiSignalsAndSlots();
    void connectOtherSignalsAndSlots();
    void disconnectOtherSignalsAndSlots();
    void reconnectOtherSignalsAndSlots();
    void reconnectAllSignalsAndSlots();
    void SetIndexLabels(bool enable);
    void SetPlaybackControls(bool enable);          // macro to enable/disable playback controls
    void TogglePlaylist();                          // toggles playlist visibility
    bool isPlaylistVisible();                       // is the playlist visible?
    void SetWindowTitle2(const QString &text);

private:
    SugoiEngine *sugoi = nullptr;
    MpvWidget *mpv = nullptr;
    ProgressIndicatorBar *fullscreenProgressIndicator = nullptr;
    bool showFullscreenIndicator = true;
#ifdef Q_OS_WIN
    FileAssoc::reg_type  regType;
    FileAssoc::reg_state regState;
    bool alwaysCheckFileAssoc = true;
#endif
    bool pauseWhenMinimized = true;
    bool osdShowLocalTime = true;
    bool allowRunInBackground = true;
    QString skinFile;
    bool playInBackground = false;

#if defined(Q_OS_WIN) && defined(QT_HAS_WINEXTRAS)
    QWinThumbnailToolBar    *thumbnail_toolbar = nullptr;
    QWinThumbnailToolButton *prev_toolbutton = nullptr,
                            *playpause_toolbutton = nullptr,
                            *next_toolbutton = nullptr;
    QWinTaskbarButton       *taskbarButton = nullptr;
    QWinTaskbarProgress     *taskbarProgress = nullptr;
    QWinJumpList            *jumplist = nullptr;
#endif
    bool            pathChanged,
                    menuVisible     = true,
                    firstItem       = false,
                    init            = false,
                    playlistState   = false;
    QTimer          *autohide       = nullptr;
    QTimer          *osdLocalTimeUpdater = nullptr;

    // variables
    QList<Recent> recent;
    Recent *current = nullptr;
    QString lang,
            onTop;
    int autoFit,
        maxRecent;
    bool hidePopup,
         remaining,
         screenshotDialog,
         debug,
         resume,
         hideAllControls = false;
    QHash<QString, QAction*> commandActionMap;

public slots:
    void setLang(QString s)          { emit langChanged(lang = std::move(s)); }
    void setOnTop(QString s)         { emit onTopChanged(onTop = std::move(s)); }
    void setAutoFit(int i)           { emit autoFitChanged(autoFit = i); }
    void setMaxRecent(int i)         { emit maxRecentChanged(maxRecent = i); }
    void setHidePopup(bool b)        { emit hidePopupChanged(hidePopup = b); }
    void setRemaining(bool b)        { emit remainingChanged(remaining = b); }
    void setScreenshotDialog(bool b) { emit screenshotDialogChanged(screenshotDialog = b); }
    void setDebug(bool b)            { emit debugChanged(debug = b); }
    void setResume(bool b)           { emit resumeChanged(resume = b); }
    void setHideAllControls(bool b)  { emit hideAllControlsChanged(hideAllControls = b); }
#ifdef Q_OS_WIN
    void setFileAssocType(const FileAssoc::reg_type t) { emit fileAssocTypeChanged(regType = t); }
    void setFileAssocState(const FileAssoc::reg_state s) { emit fileAssocStateChanged(regState = s); }
    void setAlwaysCheckFileAssoc(bool b) { emit alwaysCheckFileAssocChanged(alwaysCheckFileAssoc = b); }
#endif
    void setPauseWhenMinimized(bool b) { emit pauseWhenMinimizedChanged(pauseWhenMinimized = b); }
    void setShowFullscreenIndicator(bool b) { emit showFullscreenIndicatorChanged(showFullscreenIndicator = b); }
    void setOSDShowLocalTime(bool b)  { emit osdShowLocalTimeChanged(osdShowLocalTime = b); }
    void setAllowRunInBackground(bool b) { emit allowRunInBackgroundChanged(allowRunInBackground = b); }
    void setSkinFile(const QString &s)  { emit skinFileChanged(skinFile = s); }

signals:
    void langChanged(QString);
    void onTopChanged(QString);
    void autoFitChanged(int);
    void maxRecentChanged(int);
    void hidePopupChanged(bool);
    void remainingChanged(bool);
    void screenshotDialogChanged(bool);
    void debugChanged(bool);
    void resumeChanged(bool);
    void hideAllControlsChanged(bool);
    void openFileFromCmd(const QString &);
#ifdef Q_OS_WIN
    void fileAssocTypeChanged(FileAssoc::reg_type);
    void fileAssocStateChanged(FileAssoc::reg_state);
    void alwaysCheckFileAssocChanged(bool);
#endif
    void pauseWhenMinimizedChanged(bool);
    void showFullscreenIndicatorChanged(bool);
    void osdShowLocalTimeChanged(bool);
    void allowRunInBackgroundChanged(bool);
    void skinFileChanged(const QString &);
    void playlistChanged(const QStringList &);
};
