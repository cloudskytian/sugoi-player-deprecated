#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "widgets/cframelesswindow.h"
#include "recent.h"
#include "fileassoc.h"

#ifdef Q_OS_WIN
class QWinThumbnailToolBar;
class QWinThumbnailToolButton;
class QWinTaskbarButton;
class QWinTaskbarProgress;
class QWinJumpList;
#endif

class ProgressIndicatorBar;

namespace Ui {
class MainWindow;
}

class MainWindow : public CFramelessWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool isFullScreenMode()    { return hideAllControls || isFullScreen(); }

    Ui::MainWindow *ui = nullptr;
    QImage albumArt;

public slots:
    void MapShortcuts();
    void SetFileAssoc(FileAssoc::reg_type type = FileAssoc::reg_type::ALL, bool showUI = false);
    void BringWindowToFront();
    void initMainWindow(bool backgroundMode = false);
    void setSysTrayIconVisibility(bool v = true);

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
    void SetIndexLabels(bool enable);
    void SetPlaybackControls(bool enable);          // macro to enable/disable playback controls
    void TogglePlaylist();                          // toggles playlist visibility
    bool isPlaylistVisible();                       // is the playlist visible?
    void SetWindowTitle2(const QString &text);

private:
    ProgressIndicatorBar *fullscreenProgressIndicator = nullptr;

    bool firstShow = true;

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

    // variables
    QList<Recent> recent;
    Recent *current = nullptr;
};

#endif // MAINWINDOW_H
