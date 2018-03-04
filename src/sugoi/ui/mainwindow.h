#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringList>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QTimer>
#include <QTranslator>
#include <QHash>
#include <QAction>
#include <QRect>
#include <QMenu>
#include <QSystemTrayIcon>

#ifdef Q_OS_WIN
#include <QWinThumbnailToolBar>
#include <QWinThumbnailToolButton>
#include <QWinTaskbarButton>
#include <QWinTaskbarProgress>
#include <QWinJumpList>
#endif

#include "recent.h"
#include "widgets/logowidget.h"
#include "fileassoc.h"
#include "widgets/progressindicatorbar.h"
#include "sugoiengine.h"
#include "mpvwidget.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
friend class SugoiEngine;
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr, bool backgroundMode = false);
    ~MainWindow();

    QString getLang()          { return lang; }
    QString getOnTop()         { return onTop; }
    QString getLastDir()       { return lastDir; }
    int getAutoFit()           { return autoFit; }
    int getMaxRecent()         { return maxRecent; }
    bool getHidePopup()        { return hidePopup; }
    bool getRemaining()        { return remaining; }
    bool getScreenshotDialog() { return screenshotDialog; }
    bool getDebug()            { return debug; }
    bool getResume()           { return resume; }
    bool getHideAllControls()  { return hideAllControls; }
    bool isFullScreenMode()    { return hideAllControls || isFullScreen(); }
    FileAssoc::reg_type getFileAssocType() { return regType; }
    FileAssoc::reg_state getFileAssocState() { return regState; }
    bool getAlwaysCheckFileAssoc() { return alwaysCheckFileAssoc; }
    bool getShowFullscreenIndicator() { return showFullscreenIndicator; }
    bool getOSDShowLocalTime()   { return osdShowLocalTime; }
    bool getPauseWhenMinimized() { return pauseWhenMinimized; }
    bool getShowVideoPreview()   { return showVideoPreview; }
    bool getAllowRunInBackground() { return allowRunInBackground; }
    bool getQuickStartMode()     { return quickStartMode; }
    bool getTrayIconVisible()    { return trayIconVisible; }
    bool getAutoUpdatePlayer()   { return autoUpdatePlayer; }
    bool getAutoUpdateStreamingSupport() { return autoUpdateStreamingSupport; }
    QString getSkinFile()        { return skinFile; }
    QSystemTrayIcon *getSystemTrayIcon() { return sugoi->sysTrayIcon == nullptr ? nullptr : sugoi->sysTrayIcon; }
    bool getHwdec()              { return hwdec; }

    Ui::MainWindow *ui = nullptr;
    QImage albumArt;

public slots:
    void MapShortcuts();
    void SetFileAssoc(FileAssoc::reg_type type = FileAssoc::reg_type::ALL, bool showUI = false);
    void BringWindowToFront();

protected:
    void dragEnterEvent(QDragEnterEvent *event);    // drag file into
    void dropEvent(QDropEvent *event);              // drop file into
    void mousePressEvent(QMouseEvent *event);       // pressed mouse down
    void mouseMoveEvent(QMouseEvent *event);        // moved mouse on the form
    void mouseDoubleClickEvent(QMouseEvent *event); // double clicked the form
    bool eventFilter(QObject *obj, QEvent *event);  // event filter (get mouse move events from mpvFrame)
    void wheelEvent(QWheelEvent *event);            // the mouse wheel is used
    void keyPressEvent(QKeyEvent *event);
    void resizeEvent(QResizeEvent *event);
    void changeEvent(QEvent *event);
    void closeEvent(QCloseEvent *event);
    void showEvent(QShowEvent *event);

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
    void initMainWindow(bool backgroundMode = false);
    void SetIndexLabels(bool enable);
    void SetPlaybackControls(bool enable);          // macro to enable/disable playback controls
    void TogglePlaylist();                          // toggles playlist visibility
    bool isPlaylistVisible();                       // is the playlist visible?

private:
    SugoiEngine *sugoi = nullptr;
    MpvWidget *mpv = nullptr;
    LogoWidget *logo = nullptr;
    ProgressIndicatorBar *fullscreenProgressIndicator = nullptr;
    bool showFullscreenIndicator = true;
    FileAssoc::reg_type  regType;
    FileAssoc::reg_state regState;
    bool alwaysCheckFileAssoc = true;
    bool pauseWhenMinimized = true;
    bool osdShowLocalTime = true;
    bool showVideoPreview = true;
    bool allowRunInBackground = true;
    bool quickStartMode = true;
    bool trayIconVisible = true;
    bool autoUpdatePlayer = true;
    bool autoUpdateStreamingSupport = true;
    QString skinFile;
    bool firstShow = true;
    bool playInBackground = false;
    bool hwdec = true;

#ifdef Q_OS_WIN
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
            onTop,
            lastDir;
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
    void setLang(QString s)          { emit langChanged(lang = s); }
    void setOnTop(QString s)         { emit onTopChanged(onTop = s); }
    void setLastDir(QString s)       { emit onLastDirChanged(lastDir = s); }
    void setAutoFit(int i)           { emit autoFitChanged(autoFit = i); }
    void setMaxRecent(int i)         { emit maxRecentChanged(maxRecent = i); }
    void setHidePopup(bool b)        { emit hidePopupChanged(hidePopup = b); }
    void setRemaining(bool b)        { emit remainingChanged(remaining = b); }
    void setScreenshotDialog(bool b) { emit screenshotDialogChanged(screenshotDialog = b); }
    void setDebug(bool b)            { emit debugChanged(debug = b); }
    void setResume(bool b)           { emit resumeChanged(resume = b); }
    void setHideAllControls(bool b)  { emit hideAllControlsChanged(hideAllControls = b); }
    void setFileAssocType(const FileAssoc::reg_type t) { emit fileAssocTypeChanged(regType = t); }
    void setFileAssocState(const FileAssoc::reg_state s) { emit fileAssocStateChanged(regState = s); }
    void setAlwaysCheckFileAssoc(bool b) { emit alwaysCheckFileAssocChanged(alwaysCheckFileAssoc = b); }
    void setPauseWhenMinimized(bool b) { emit pauseWhenMinimizedChanged(pauseWhenMinimized = b); }
    void setShowFullscreenIndicator(bool b) { emit showFullscreenIndicatorChanged(showFullscreenIndicator = b); }
    void setOSDShowLocalTime(bool b)  { emit osdShowLocalTimeChanged(osdShowLocalTime = b); }
    void setShowVideoPreview(bool b)  { emit showVideoPreviewChanged(showVideoPreview = b); }
    void setAllowRunInBackground(bool b) { emit allowRunInBackgroundChanged(allowRunInBackground = b); }
    void setQuickStartMode(bool b)    { emit quickStartModeChanged(quickStartMode = b); }
    void setTrayIconVisible(bool b)   { emit trayIconVisibleChanged(trayIconVisible = b); }
    void setAutoUpdatePlayer(bool b)  { emit autoUpdatePlayerChanged(autoUpdatePlayer = b); }
    void setAutoUpdateStreamingSupport(bool b) { emit autoUpdateStreamingSupportChanged(autoUpdateStreamingSupport = b); }
    void setSkinFile(const QString &s)  { emit skinFileChanged(skinFile = s); }
    void setHwdec(bool b)             { emit hwdecChanged(hwdec = b); }

signals:
    void langChanged(QString);
    void onTopChanged(QString);
    void onLastDirChanged(QString);
    void autoFitChanged(int);
    void maxRecentChanged(int);
    void hidePopupChanged(bool);
    void remainingChanged(bool);
    void screenshotDialogChanged(bool);
    void debugChanged(bool);
    void resumeChanged(bool);
    void hideAllControlsChanged(bool);
    void openFileFromCmd(const QString &);
    void fileAssocTypeChanged(const FileAssoc::reg_type);
    void fileAssocStateChanged(const FileAssoc::reg_state);
    void alwaysCheckFileAssocChanged(bool);
    void pauseWhenMinimizedChanged(bool);
    void showFullscreenIndicatorChanged(bool);
    void osdShowLocalTimeChanged(bool);
    void showVideoPreviewChanged(bool);
    void allowRunInBackgroundChanged(bool);
    void quickStartModeChanged(bool);
    void trayIconVisibleChanged(bool);
    void autoUpdatePlayerChanged(bool);
    void autoUpdateStreamingSupportChanged(bool);
    void skinFileChanged(const QString &);
    void playlistChanged(const QStringList &);
    void hwdecChanged(bool);
};

#endif // MAINWINDOW_H
