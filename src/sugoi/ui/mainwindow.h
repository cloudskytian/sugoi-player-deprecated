#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "widgets/cframelesswindow.h"

#ifdef Q_OS_WIN
class QWinThumbnailToolBar;
class QWinThumbnailToolButton;
class QWinTaskbarButton;
class QWinTaskbarProgress;
class QWinJumpList;
#endif

namespace Ui {
class MainWindow;
}

class MainWindow : public CFramelessWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public:
    Ui::MainWindow *ui = nullptr;

public slots:
    void initMainWindow(bool backgroundMode = false);
    void setWindowTitle2(const QString &text);

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

private:
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
};

#endif // MAINWINDOW_H
