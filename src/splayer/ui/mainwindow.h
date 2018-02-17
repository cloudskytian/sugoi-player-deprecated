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

#if defined(Q_OS_WIN)
#include <QWinThumbnailToolBar>
#include <QWinThumbnailToolButton>
#include <QWinTaskbarButton>
#include <QWinTaskbarProgress>
#endif

#include "recent.h"
#include "widgets/logowidget.h"
#include "fileassoc.h"
#include "widgets/sprogressindicatorbar.h"

namespace Ui {
class MainWindow;
}

class BakaEngine;
class MpvHandler;

class MainWindow : public QMainWindow
{
friend class BakaEngine;
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
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
    QMenu *getContextMenu()    { return contextMenu; }
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

    Ui::MainWindow  *ui;
    QImage albumArt;

public slots:
    void Load(const QString &f = QString(), bool backgroundMode = false);
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
    void SetIndexLabels(bool enable);
    void SetPlaybackControls(bool enable);          // macro to enable/disable playback controls
    void TogglePlaylist();                          // toggles playlist visibility
    bool isPlaylistVisible();                       // is the playlist visible?

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

private:
    BakaEngine      *baka;
    MpvHandler      *mpv;
    LogoWidget      *logo;
    QMenu           *contextMenu;
    SProgressIndicatorBar *fullscreenProgressIndicator = nullptr;
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

#if defined(Q_OS_WIN)
    QWinThumbnailToolBar    *thumbnail_toolbar;
    QWinThumbnailToolButton *prev_toolbutton,
                            *playpause_toolbutton,
                            *next_toolbutton;
    QWinTaskbarButton       *taskbarButton;
    QWinTaskbarProgress     *taskbarProgress;
#endif
    bool            pathChanged,
                    menuVisible,
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
};

#endif // MAINWINDOW_H
