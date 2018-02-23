#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtMath>
#include <QLibraryInfo>
#include <QMimeData>
#include <QMimeDatabase>
#include <QDesktopWidget>
#include <QWinJumpList>
#include <QtWinExtras>
#include <QtWin>
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>
#include <QMessageBox>
#include <QTime>
#include <QUrl>
#include <QCursor>
#include <QtConcurrent>

#include "sugoiengine.h"
#include "mpvhandler.h"
#include "overlayhandler.h"
#include "util.h"
#include "widgets/dimdialog.h"
#include "inputdialog.h"
#include "screenshotdialog.h"
#include "winsparkle.h"
#include "skinmanager.h"

MainWindow::MainWindow(QWidget *parent, bool backgroundMode):
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

#if defined(Q_OS_UNIX) || defined(Q_OS_LINUX)
    // update streaming support disabled on unix platforms
    ui->actionUpdate_Streaming_Support->setEnabled(false);
#endif

    ShowPlaylist(false);
    addActions(ui->menubar->actions()); // makes menubar shortcuts work even when menubar is hidden

    // initialize managers/handlers
    if (sugoi != nullptr)
    {
        delete sugoi;
        sugoi = nullptr;
        mpv = nullptr;
    }
    sugoi = new SugoiEngine(this);
    mpv = sugoi->mpv;

    ui->playlistWidget->AttachEngine(sugoi);
    ui->mpvFrame->installEventFilter(this); // capture events on mpvFrame in the eventFilter function
    ui->mpvFrame->setMouseTracking(true);
    if (autohide != nullptr)
    {
        delete autohide;
        autohide = nullptr;
    }
    autohide = new QTimer(this);
    if (osdLocalTimeUpdater != nullptr)
    {
        delete osdLocalTimeUpdater;
        osdLocalTimeUpdater = nullptr;
    }
    osdLocalTimeUpdater = new QTimer(this);
    if (logo != nullptr)
    {
        delete logo;
        logo = nullptr;
    }
    logo = new LogoWidget(this);
    logo->setGeometry(0, menuBar()->height() / 2, width(),
                height() - menuBar()->height() / 2 - ui->seekBar->height() - ui->playbackLayoutWidget->height());
    logo->show();

    ui->playlistButton->setEnabled(true);

    connectOtherSignalsAndSlots();
    connectMpvSignalsAndSlots();
    connectUiSignalsAndSlots();

    // add multimedia shortcuts
    ui->action_Play->setShortcuts({ui->action_Play->shortcut(), QKeySequence(Qt::Key_MediaPlay)});
    ui->action_Stop->setShortcuts({ui->action_Stop->shortcut(), QKeySequence(Qt::Key_MediaStop)});
    ui->actionPlay_Next_File->setShortcuts({ui->actionPlay_Next_File->shortcut(), QKeySequence(Qt::Key_MediaNext)});
    ui->actionPlay_Previous_File->setShortcuts({ui->actionPlay_Previous_File->shortcut(), QKeySequence(Qt::Key_MediaPrevious)});

    setContextMenuPolicy(Qt::ActionsContextMenu);

    Load(backgroundMode);
}

MainWindow::~MainWindow()
{
    win_sparkle_cleanup();

    if(current != nullptr)
    {
        int t = mpv->getTime(),
            l = mpv->getFileInfo().length;
        if(t > 0.05*l && t < 0.95*l) // only save if within the middle 90%
            current->time = t;
        else
            current->time = 0;
    }

    sugoi->SaveSettings();

#if defined(Q_OS_WIN)
    delete prev_toolbutton;
    delete playpause_toolbutton;
    delete next_toolbutton;
    delete thumbnail_toolbar;
    delete taskbarProgress;
    delete taskbarButton;
#endif
    delete sugoi;
    delete ui;
}

void MainWindow::Load(bool backgroundMode)
{
    // load the settings here--the constructor has already been called
    // this solves some issues with setting things before the constructor has ended
    menuVisible = true/*ui->menubar->isVisible()*/; // does the OS use a menubar? (appmenu doesn't)
#if defined(Q_OS_WIN)
    QtWin::enableBlurBehindWindow(this);

    QWinJumpList *jumplist = new QWinJumpList(this);
    jumplist->recent()->setVisible(true);

    taskbarButton = new QWinTaskbarButton(this);
    taskbarButton->setWindow(this->windowHandle());

    taskbarProgress = taskbarButton->progress();
    taskbarProgress->setMinimum(0);
    taskbarProgress->setMaximum(1000);

    // add windows 7+ thubnail toolbar buttons
    thumbnail_toolbar = new QWinThumbnailToolBar(this);
    thumbnail_toolbar->setWindow(this->windowHandle());

    prev_toolbutton = new QWinThumbnailToolButton(thumbnail_toolbar);
    prev_toolbutton->setEnabled(false);
    prev_toolbutton->setToolTip(tr("Previous"));
    prev_toolbutton->setIcon(QIcon(":/images/tool-previous.ico"));
    connect(prev_toolbutton, &QWinThumbnailToolButton::clicked,
            [=]
            {
                ui->playlistWidget->PlayIndex(-1, true);
            });

    playpause_toolbutton = new QWinThumbnailToolButton(thumbnail_toolbar);
    playpause_toolbutton->setEnabled(false);
    playpause_toolbutton->setToolTip(tr("Play"));
    playpause_toolbutton->setIcon(QIcon(":/images/tool-play.ico"));
    connect(playpause_toolbutton, &QWinThumbnailToolButton::clicked,
            [=]
            {
                sugoi->PlayPause();
            });

    next_toolbutton = new QWinThumbnailToolButton(thumbnail_toolbar);
    next_toolbutton->setEnabled(false);
    next_toolbutton->setToolTip(tr("Next"));
    next_toolbutton->setIcon(QIcon(":/images/tool-next.ico"));
    connect(next_toolbutton, &QWinThumbnailToolButton::clicked,
            [=]
            {
                ui->playlistWidget->PlayIndex(1, true);
            });

    thumbnail_toolbar->addButton(prev_toolbutton);
    thumbnail_toolbar->addButton(playpause_toolbutton);
    thumbnail_toolbar->addButton(next_toolbutton);
#endif
    sugoi->LoadSettings();
    mpv->Initialize();

    if (osdShowLocalTime)
    {
        if (osdLocalTimeUpdater)
        {
            if (osdLocalTimeUpdater->isActive())
            {
                osdLocalTimeUpdater->stop();
            }
            osdLocalTimeUpdater->start(1000);
        }
    }

    if (!backgroundMode)
    {
        FileAssoc fileAssoc;
        setFileAssocState(fileAssoc.getMediaFilesRegisterState());
        if (getAlwaysCheckFileAssoc())
        {
            if (getFileAssocType() != FileAssoc::reg_type::NONE && getFileAssocState() != FileAssoc::reg_state::ALL_REGISTERED)
            {
                SetFileAssoc(getFileAssocType(), true);
            }
        }
    }
    else
    {
        this->hide();
    }
}

void MainWindow::MapShortcuts()
{
    auto tmp = commandActionMap;
    // map shortcuts to actions
    for(auto input_iter = sugoi->input.begin(); input_iter != sugoi->input.end(); ++input_iter)
    {
        auto commandAction = tmp.find(input_iter->first);
        if(commandAction != tmp.end())
        {
            (*commandAction)->setShortcut(QKeySequence(input_iter.key()));
            tmp.erase(commandAction);
        }
    }
    // clear the rest
    for(auto iter = tmp.begin(); iter != tmp.end(); ++iter)
        (*iter)->setShortcut(QKeySequence());
}

void MainWindow::SetFileAssoc(FileAssoc::reg_type type, bool showUI)
{
    const QString path = QCoreApplication::applicationFilePath();
    QString param = QString::fromLatin1("--regall");
    if (type == FileAssoc::reg_type::VIDEO_ONLY)
    {
        param = QString::fromLatin1("--regvideo");
    }
    else if (type == FileAssoc::reg_type::AUDIO_ONLY)
    {
        param = QString::fromLatin1("--regaudio");
    }
    else if (type == FileAssoc::reg_type::NONE)
    {
        param = QString::fromLatin1("--unregall");
    }
    bool needChange = true;
    if (showUI)
    {
        if (QMessageBox::question(this, tr("Associate media files"), tr("You have configured Sugoi Player to check "
             "file associations every time Sugoi Player starts up. And now Sugoi Player found that it is not associated "
             "with some/all media files. Do you want to associate them now?")) == QMessageBox::No)
        {
            needChange = false;
        }
    }
    if (needChange)
    {
        if (Util::executeProgramWithAdministratorPrivilege(path, param))
        {
            setFileAssocType(type);
            FileAssoc fileAssoc;
            setFileAssocState(fileAssoc.getMediaFilesRegisterState());
        }
    }
}

void MainWindow::BringWindowToFront()
{
    if (isHidden())
    {
        show();
    }
    showMinimized();
    setWindowState(windowState() & ~Qt::WindowMinimized);
    raise();
    activateWindow();
}

static bool canHandleDrop(const QDragEnterEvent *event)
{
    const QList<QUrl> urls = event->mimeData()->urls();
    if (urls.size() < 1)
    {
        return false;
    }
    QMimeDatabase mimeDatabase;
    return Util::supportedMimeTypes().
        contains(mimeDatabase.mimeTypeForUrl(urls.constFirst()).name());
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    event->setAccepted(canHandleDrop(event));
    QMainWindow::dragEnterEvent(event);
}

void MainWindow::dropEvent(QDropEvent *event)
{
    event->accept();
    QUrl filePath = event->mimeData()->urls().constFirst();
    if (filePath.isLocalFile())
    {
        mpv->LoadFile(filePath.toLocalFile());
    }
    else
    {
        mpv->LoadFile(filePath.url());
    }
    QMainWindow::dropEvent(event);
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        if(ui->remainingLabel->rect().contains(ui->remainingLabel->mapFrom(this, event->pos()))) // clicked timeLayoutWidget
        {
            setRemaining(!remaining); // todo: use a sugoicommand
        }
        else if (ui->mpvFrame->geometry().contains(event->pos())) // mouse is in the mpvFrame
        {
            mpv->PlayPause(ui->playlistWidget->CurrentItem());
        }
    }
    QMainWindow::mousePressEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if(isFullScreenMode())
    {
        setCursor(QCursor(Qt::ArrowCursor)); // show the cursor
        autohide->stop();

        QRect playbackRect = geometry();
        playbackRect.setTop(playbackRect.bottom() - 60);
        bool showPlayback = playbackRect.contains(event->globalPos());
        ui->playbackLayoutWidget->setVisible(showPlayback || ui->outputTextEdit->isVisible());
        ui->seekBar->setVisible(showPlayback || ui->outputTextEdit->isVisible());

        QRect playlistRect = geometry();
        playlistRect.setLeft(playlistRect.right() - qCeil(playlistRect.width()/7.0));
        bool showPlaylist = playlistRect.contains(event->globalPos());
        ShowPlaylist(showPlaylist);

        if (fullscreenProgressIndicator)
        {
            fullscreenProgressIndicator->setVisible(!ui->playbackLayoutWidget->isVisible());
        }

        if(!(showPlayback || showPlaylist) && autohide)
            autohide->start(500);
    }
    QMainWindow::mouseMoveEvent(event);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if(obj == ui->mpvFrame && isFullScreenMode() && event->type() == QEvent::MouseMove)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        mouseMoveEvent(mouseEvent);
    }
    else if(event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        keyPressEvent(keyEvent);
    }
    return false;
}

void MainWindow::wheelEvent(QWheelEvent *event)
{
    if(event->delta() > 0)
        mpv->Volume(mpv->getVolume()+5, true);
    else
        mpv->Volume(mpv->getVolume()-5, true);
    QMainWindow::wheelEvent(event);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    // keyboard shortcuts
    if(!sugoi->input.empty())
    {
        QString key = QKeySequence(event->modifiers()|event->key()).toString();

        // TODO: Add more protection/find a better way to protect edit boxes from executing commands
        if(focusWidget() == ui->inputLineEdit &&
           key == "Return")
            return;

        // Escape exits fullscreen
        if(isFullScreen() &&
           key == "Esc") {
            FullScreen(false);
            return;
        }

        // find shortcut in input hash table
        auto iter = sugoi->input.find(key);
        if(iter != sugoi->input.end())
            sugoi->Command(iter->first); // execute command
    }
    QMainWindow::keyPressEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    if(ui->actionMedia_Info->isChecked())
        sugoi->overlay->showInfoText();
    if (!isPlaylistVisible())
    {
        logo->setGeometry(0, menuBar()->height() / 2, width(),
                    height() - menuBar()->height() / 2 - ui->seekBar->height() - ui->playbackLayoutWidget->height());
    }
    else
    {
        logo->setGeometry(0, menuBar()->height() / 2, width() - ui->splitter->position(),
                height() - menuBar()->height() / 2 - ui->seekBar->height() - ui->playbackLayoutWidget->height());
    }
    if (hideAllControls)
    {
        if (fullscreenProgressIndicator)
        {
            fullscreenProgressIndicator->setGeometry(0, height() - fullscreenProgressIndicator->height(),
                                                     width(), fullscreenProgressIndicator->height());
        }
        ui->playbackLayoutWidget->setGeometry(0, height() - ui->playbackLayoutWidget->height(),
                                              width(), ui->playbackLayoutWidget->height());
        ui->seekBar->setGeometry(0, height() - ui->playbackLayoutWidget->height() - ui->seekBar->height(),
                                 width(), ui->seekBar->height());
    }
    QMainWindow::resizeEvent(event);
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange)
    {
        if (windowState() == Qt::WindowMinimized)
        {
            if (pauseWhenMinimized)
            {
                if (IsPlayingVideo(mpv->getFileFullPath()))
                {
                    mpv->Pause();
                }
            }
        }
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    sugoi->SaveSettings();
    bool canClose = true;
    if (IsPlayingMusic(mpv->getFileFullPath()))
    {
        if (allowRunInBackground)
        {
            canClose = false;
        }
        else if (QMessageBox::question(this, tr("Exit"), tr("Do you want Sugoi Player to run in background?")) == QMessageBox::Yes)
        {
            canClose = false;
        }
    }
    if (!canClose)
    {
        hide();
        if (sugoi->sysTrayIcon->isVisible())
        {
            sugoi->sysTrayIcon->showMessage(QString::fromLatin1("Sugoi Player"), tr("Sugoi Player is running in background now, click the trayicon to bring Sugoi Player to foreground."), QIcon(":/images/player.svg"), 4000);
        }
        event->ignore();
        return;
    }
    if (quickStartMode)
    {
        closing = true;
        QTimer::singleShot(0, [=]
        {
            showMinimized();
            if (logo != nullptr)
            {
                if (logo->isHidden())
                {
                    logo->show();
                }
            }
            sugoi->sysTrayIcon->hide();
            if (sugoi != nullptr)
            {
                delete sugoi;
                sugoi = nullptr;
                mpv = nullptr;
            }
            sugoi = new SugoiEngine(this);
            mpv = sugoi->mpv;
            reconnectAllSignalsAndSlots();
            ui->playlistWidget->AttachEngine(sugoi);
            sugoi->LoadSettings();
            mpv->Initialize();
        });
        QTimer::singleShot(1000, [=]
        {
            hide();
        });
        event->ignore();
        return;
    }
    event->accept();
    QMainWindow::closeEvent(event);
}

void MainWindow::hideEvent(QHideEvent *event)
{
    QMainWindow::hideEvent(event);
    closing = false;
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    if (sugoi->sysTrayIcon != nullptr)
    {
        if (trayIconVisible && !sugoi->sysTrayIcon->isVisible())
        {
            sugoi->sysTrayIcon->show();
        }
        else if (!trayIconVisible && sugoi->sysTrayIcon->isVisible())
        {
            sugoi->sysTrayIcon->hide();
        }
    }
#ifdef _DEBUG
    firstShow = false;
    return;
#endif
    if (firstShow)
    {
        win_sparkle_set_appcast_url("https://raw.githubusercontent.com/wangwenx190/Sugoi-Player/master/src/player/appcast.xml");
        win_sparkle_set_lang(lang.toUtf8().constData());
        win_sparkle_init();
    }
    if (autoUpdatePlayer)
    {
        win_sparkle_check_update_without_ui();
    }
//    if (autoUpdateStreamingSupport)
//    {
//        ui->actionUpdate_Streaming_Support->triggered();
//    }
    firstShow = false;
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton && ui->mpvFrame->geometry().contains(event->pos())) // if mouse is in the mpvFrame
    {
        if(!isFullScreen() && ui->action_Full_Screen->isEnabled()) // don't allow people to go full screen if they shouldn't be able to
            FullScreen(true);
        else // they can leave fullscreen even if it's disabled (eg. video ends while in full screen)
            FullScreen(false);
        event->accept();
    }
    QMainWindow::mouseDoubleClickEvent(event);
}

void MainWindow::SetIndexLabels(bool enable)
{
    int i = ui->playlistWidget->currentRow(),
        index = ui->playlistWidget->CurrentIndex();

    // next file
    if(enable && index+1 < ui->playlistWidget->count()) // not the last entry
    {
        SetNextButtonEnabled(true);
        ui->nextButton->setIndex(index+2); // starting at 1 instead of at 0 like actual index

    }
    else
        SetNextButtonEnabled(false);

    // previous file
    if(enable && index-1 >= 0) // not the first entry
    {
        SetPreviousButtonEnabled(true);
        ui->previousButton->setIndex(-index); // we use a negative index value for the left button
    }
    else
        SetPreviousButtonEnabled(false);

    if(i == -1) // no selection
    {
        ui->indexLabel->setText(tr("No selection"));
        ui->indexLabel->setEnabled(false);
    }
    else
    {
        ui->indexLabel->setEnabled(true);
        ui->indexLabel->setText(tr("%0 / %1").arg(QString::number(i+1), QString::number(ui->playlistWidget->count())));
    }
}

void MainWindow::SetPlaybackControls(bool enable)
{
    // playback controls
    ui->seekBar->setEnabled(enable);
    ui->rewindButton->setEnabled(enable);

    SetIndexLabels(enable);

    // menubar
    ui->action_Stop->setEnabled(enable);
    ui->action_Restart->setEnabled(enable);
    ui->menuS_peed->setEnabled(enable);
    ui->action_Jump_to_Time->setEnabled(enable);
    ui->actionMedia_Info->setEnabled(enable);
    ui->actionShow_in_Folder->setEnabled(enable && sugoi->mpv->getPath() != QString());
    ui->action_Full_Screen->setEnabled(enable);
    if(!enable)
    {
        ui->action_Hide_Album_Art->setEnabled(false);
        ui->menuSubtitle_Track->setEnabled(false);
        ui->menuFont_Si_ze->setEnabled(false);
    }
}

void MainWindow::HideAllControls(bool w, bool s)
{
    if(s)
    {
        hideAllControls = w;
        if(isFullScreen())
            return;
    }
    if(w)
    {
        if(s || !hideAllControls)
            playlistState = ui->playlistLayoutWidget->isVisible();
        ui->menubar->setVisible(false);

        ui->playbackLayoutWidget->hide();
        ui->seekBar->hide();
        ui->centralwidget->layout()->removeWidget(ui->playbackLayoutWidget);
        ui->playbackLayoutWidget->setParent(this);
        ui->playbackLayoutWidget->move(QPoint(0, geometry().height() - ui->playbackLayoutWidget->height()));
        ui->centralwidget->layout()->removeWidget(ui->seekBar);
        ui->seekBar->setParent(this);
        ui->seekBar->move(QPoint(0, geometry().height() - ui->playbackLayoutWidget->height() - ui->seekBar->height()));

        if (fullscreenProgressIndicator)
        {
            fullscreenProgressIndicator->close();
            delete fullscreenProgressIndicator;
            fullscreenProgressIndicator = nullptr;
        }
        if (showFullscreenIndicator)
        {
            fullscreenProgressIndicator = new ProgressIndicatorBar(this);
            fullscreenProgressIndicator->setFixedSize(QSize(width(), (2.0 / 1080.0) * height()));
            fullscreenProgressIndicator->move(QPoint(0, height() - fullscreenProgressIndicator->height()));
            fullscreenProgressIndicator->setRange(0, 1000);
            fullscreenProgressIndicator->show();
        }

        mouseMoveEvent(new QMouseEvent(QMouseEvent::MouseMove,
                                       QCursor::pos(),
                                       Qt::NoButton,Qt::NoButton,Qt::NoModifier));
    }
    else
    {
        if (fullscreenProgressIndicator)
        {
            fullscreenProgressIndicator->close();
            delete fullscreenProgressIndicator;
            fullscreenProgressIndicator = nullptr;
        }

        ui->seekBar->setParent(centralWidget());
        ui->centralwidget->layout()->addWidget(ui->seekBar);
        ui->playbackLayoutWidget->setParent(centralWidget());
        ui->centralwidget->layout()->addWidget(ui->playbackLayoutWidget);

        if(menuVisible)
            ui->menubar->setVisible(true);
        ui->seekBar->setVisible(true);
        ui->playbackLayoutWidget->setVisible(true);
        setCursor(QCursor(Qt::ArrowCursor)); // show cursor
        autohide->stop();
        ShowPlaylist(playlistState);
    }
}

void MainWindow::FullScreen(bool fs)
{
    if(fs)
    {
        if(sugoi->dimDialog && sugoi->dimDialog->isVisible())
            sugoi->Dim(false);
        setWindowState(windowState() | Qt::WindowFullScreen);
        if(!hideAllControls)
            HideAllControls(true, false);
    }
    else
    {
        setWindowState(windowState() & ~Qt::WindowFullScreen);
        if(!hideAllControls)
            HideAllControls(false, false);
    }
}

bool MainWindow::isPlaylistVisible()
{
    // if the position is 0, playlist is hidden
    return (ui->splitter->position() != 0);
}

void MainWindow::TogglePlaylist()
{
    ShowPlaylist(!isPlaylistVisible());
}

void MainWindow::ShowPlaylist(bool visible)
{
    if(ui->splitter->position() != 0 && visible) // ignore showing if it's already visible as it resets original position
        return;

    if(visible)
    {
        ui->splitter->setPosition(ui->splitter->normalPosition()); // bring splitter position to normal
        logo->setGeometry(0, menuBar()->height() / 2, width() - ui->splitter->normalPosition(),
                height() - menuBar()->height() / 2 - ui->seekBar->height() - ui->playbackLayoutWidget->height());
    }
    else
    {
        if(ui->splitter->position() != ui->splitter->max() && ui->splitter->position() != 0)
            ui->splitter->setNormalPosition(ui->splitter->position()); // save current splitter position as the normal position
        ui->splitter->setPosition(0); // set splitter position to right-most
        setFocus();
//        logo->setGeometry(0, menuBar()->height() / 2, width(),
//                height() - menuBar()->height() / 2 - ui->seekBar->height() - ui->playbackLayoutWidget->height());
    }
}

void MainWindow::HideAlbumArt(bool hide)
{
    if(hide)
    {
        if(ui->splitter->position() != ui->splitter->max() && ui->splitter->position() != 0)
            ui->splitter->setNormalPosition(ui->splitter->position()); // save splitter position as the normal position
        ui->splitter->setPosition(ui->splitter->max()); // bring the splitter position to the left-most
    }
    else
        ui->splitter->setPosition(ui->splitter->normalPosition()); // bring the splitter to normal position
}

void MainWindow::UpdateRecentFiles()
{
    ui->menu_Recently_Opened->clear();
    QAction *action;
    int n = 1,
        N = recent.length();
    for(auto &f : recent)
    {
        action = ui->menu_Recently_Opened->addAction(QString("%0. %1").arg(Util::FormatNumberWithAmpersand(n, N), Util::ShortenPathToParent(f).replace("&","&&")));
        if(n++ == 1)
            action->setShortcut(QKeySequence("Ctrl+Z"));
        connect(action, &QAction::triggered,
                [=]
                {
                    mpv->LoadFile(f);
                });
    }
}

void MainWindow::SetNextButtonEnabled(bool enable)
{
    ui->nextButton->setEnabled(enable);
    ui->actionPlay_Next_File->setEnabled(enable);
#if defined(Q_OS_WIN)
    next_toolbutton->setEnabled(enable);
#endif
}

void MainWindow::SetPreviousButtonEnabled(bool enable)
{
    ui->previousButton->setEnabled(enable);
    ui->actionPlay_Previous_File->setEnabled(enable);
#if defined(Q_OS_WIN)
    prev_toolbutton->setEnabled(enable);
#endif
}

void MainWindow::SetPlayButtonIcon(bool play)
{
    if(play)
    {
        ui->playButton->setIcon(QIcon(":/images/default_play.svg"));
        ui->action_Play->setText(tr("&Play"));
#if defined(Q_OS_WIN)
        playpause_toolbutton->setToolTip(tr("Play"));
        playpause_toolbutton->setIcon(QIcon(":/images/tool-play.ico"));
        taskbarButton->setOverlayIcon(QIcon(":/images/tool-pause.ico"));
        taskbarProgress->show();
        taskbarProgress->pause();
#endif
    }
    else // pause icon
    {
        ui->playButton->setIcon(QIcon(":/images/default_pause.svg"));
        ui->action_Play->setText(tr("&Pause"));
#if defined(Q_OS_WIN)
        playpause_toolbutton->setToolTip(tr("Pause"));
        playpause_toolbutton->setIcon(QIcon(":/images/tool-pause.ico"));
        taskbarButton->setOverlayIcon(QIcon(":/images/tool-play.ico"));
        taskbarProgress->show();
        taskbarProgress->resume();
#endif
    }
}

void MainWindow::SetRemainingLabels(int time)
{
    // todo: move setVisible functions outside of this function which gets called every second and somewhere at the start of a video
    const Mpv::FileInfo &fi = mpv->getFileInfo();
    if(fi.length == 0)
    {
        ui->durationLabel->setText(Util::FormatTime(time, time));
        ui->remainingLabel->setVisible(false);
        ui->seperatorLabel->setVisible(false);
    }
    else
    {
        ui->remainingLabel->setVisible(true);
        ui->seperatorLabel->setVisible(true);

        ui->durationLabel->setText(Util::FormatTime(time, fi.length));
        if(remaining)
        {
            int remainingTime = fi.length - time;
            QString text = "-" + Util::FormatTime(remainingTime, fi.length);
            if(mpv->getSpeed() != 1)
            {
                double speed = mpv->getSpeed();
                text += QString("  (-%0)").arg(Util::FormatTime(int(remainingTime/speed), int(fi.length/speed)));
            }
            ui->remainingLabel->setText(text);
        }
        else
        {
            QString text = Util::FormatTime(fi.length, fi.length);
            if(mpv->getSpeed() != 1)
            {
                double speed = mpv->getSpeed();
                text += QString("  (%0)").arg(Util::FormatTime(int(fi.length/speed), int(fi.length/speed)));
            }
            ui->remainingLabel->setText(text);
        }
    }
}

bool MainWindow::IsPlayingMusic(const QString &filePath)
{
    if (mpv->getPlayState() > 0)
    {
        QFileInfo fi(filePath);
        QString suffix = QString::fromLatin1("*.") + fi.suffix();
        if (Mpv::audio_filetypes.contains(suffix))
        {
            return true;
        }
    }
    return false;
}

bool MainWindow::IsPlayingVideo(const QString &filePath)
{
    if (mpv->getPlayState() > 0)
    {
        QFileInfo fi(filePath);
        QString suffix = QString::fromLatin1("*.") + fi.suffix();
        if (Mpv::video_filetypes.contains(suffix))
        {
            return true;
        }
    }
    return false;
}

void MainWindow::connectMpvSignalsAndSlots()
{
    connect(mpv, &MpvHandler::playlistChanged, this, &MainWindow::playlistChanged);

    connect(mpv, &MpvHandler::fileInfoChanged,
            [=](const Mpv::FileInfo &fileInfo)
            {
                if(mpv->getPlayState() > 0)
                {
                    if(fileInfo.media_title == "")
                        setWindowTitle("Sugoi Player");
                    else if(fileInfo.media_title == "-")
                        setWindowTitle("Sugoi Player: stdin"); // todo: disable playlist?
                    else
                        setWindowTitle(fileInfo.media_title);

                    QString f = mpv->getFile(), file = mpv->getPath()+f;
                    if(f != QString() && maxRecent > 0)
                    {
                        int i = recent.indexOf(file);
                        if(i >= 0)
                        {
                            int t = recent.at(i).time;
                            if(t > 0 && resume)
                                mpv->Seek(t);
                            recent.removeAt(i);
                        }
                        if(recent.isEmpty() || recent.front() != file)
                        {
                            UpdateRecentFiles(); // update after initialization and only if the current file is different from the first recent
                            while(recent.length() > maxRecent-1)
                                recent.removeLast();
                            recent.push_front(
                                Recent(file,
                                       (mpv->getPath() == QString() || !Util::IsValidFile(file)) ?
                                           fileInfo.media_title : QString()));
                            current = &recent.front();
                        }
                    }

                    // reset speed if length isn't known and we have a streaming video
                    // todo: don't save this reset, put their speed back when a normal video comes on
                    // todo: disable speed alteration during streaming media
                    if(fileInfo.length == 0)
                        if(mpv->getSpeed() != 1)
                            mpv->Speed(1);

                    ui->seekBar->setTracking(fileInfo.length);

                    if(ui->actionMedia_Info->isChecked())
                        sugoi->MediaInfo(true);

                    SetRemainingLabels(fileInfo.length);

                    if(sugoi->sysTrayIcon->isVisible() && !hidePopup)
                    {
                        QString title = QString();
                        if (mpv->getFileInfo().metadata.contains("artist"))
                        {
                            title = mpv->getFileInfo().metadata["artist"];
                        }
                        if (title.isEmpty())
                        {
                            if (mpv->getFileInfo().metadata.contains("author"))
                            {
                                title = mpv->getFileInfo().metadata["author"];
                            }
                        }
                        sugoi->sysTrayIcon->showMessage(title, mpv->getFileInfo().media_title, QIcon(":/images/player.svg"), 4000);
                    }
                }
            });

    connect(mpv, &MpvHandler::trackListChanged,
            [=](const QList<Mpv::Track> &trackList)
            {
                if(mpv->getPlayState() > 0)
                {
                    QAction *action;
                    bool video = false,
                         albumArt = false;

                    ui->menuSubtitle_Track->clear();
                    ui->menuSubtitle_Track->addAction(ui->action_Add_Subtitle_File);
                    ui->menuAudio_Tracks->clear();
                    ui->menuAudio_Tracks->addAction(ui->action_Add_Audio_File);
                    for(auto &track : trackList)
                    {
                        if(track.type == "sub")
                        {
                            action = ui->menuSubtitle_Track->addAction(QString("%0: %1 (%2)").arg(QString::number(track.id), track.title, track.lang + (track.external ? "*" : "")).replace("&", "&&"));
                            connect(action, &QAction::triggered,
                                    [=]
                                    {
                                        // basically, if you uncheck the selected subtitle id, we hide subtitles
                                        // when you check a subtitle id, we make sure subtitles are showing and set it
                                        if(mpv->getSid() == track.id)
                                        {
                                            if(mpv->getSubtitleVisibility())
                                            {
                                                mpv->ShowSubtitles(false);
                                                return;
                                            }
                                            else
                                                mpv->ShowSubtitles(true);
                                        }
                                        else if(!mpv->getSubtitleVisibility())
                                            mpv->ShowSubtitles(true);
                                        mpv->Sid(track.id);
                                        mpv->ShowText(QString("%0 %1: %2 (%3)").arg(tr("Sub"), QString::number(track.id), track.title, track.lang + (track.external ? "*" : "")));
                                    });
                        }
                        else if(track.type == "audio")
                        {
                            action = ui->menuAudio_Tracks->addAction(QString("%0: %1 (%2)").arg(QString::number(track.id), track.title, track.lang).replace("&", "&&"));
                            connect(action, &QAction::triggered,
                                    [=]
                                    {
                                        if(mpv->getAid() != track.id) // don't allow selection of the same track
                                        {
                                            mpv->Aid(track.id);
                                            mpv->ShowText(QString("%0 %1: %2 (%3)").arg(tr("Audio"), QString::number(track.id), track.title, track.lang));
                                        }
                                        else
                                            action->setChecked(true); // recheck the track
                                    });
                        }
                        else if(track.type == "video") // video track
                        {
                            if(!track.albumart) // isn't album art
                                video = true;
                            else
                                albumArt = true;
                        }
                    }
                    if(video)
                    {
                        // if we were hiding album art, show it--we've gone to a video
                        if(ui->mpvFrame->styleSheet() != QString()) // remove filler album art
                            ui->mpvFrame->setStyleSheet("");
                        if(ui->action_Hide_Album_Art->isChecked())
                            HideAlbumArt(false);
                        ui->action_Hide_Album_Art->setEnabled(false);
                        ui->menuSubtitle_Track->setEnabled(true);
                        if(ui->menuSubtitle_Track->actions().count() > 1)
                        {
                            ui->menuFont_Si_ze->setEnabled(true);
                            ui->actionShow_Subtitles->setEnabled(true);
                            ui->actionShow_Subtitles->setChecked(mpv->getSubtitleVisibility());
                        }
                        else
                        {
                            ui->menuFont_Si_ze->setEnabled(false);
                            ui->actionShow_Subtitles->setEnabled(false);
                            ui->actionShow_Subtitles->setChecked(false);
                        }
                        ui->menuTake_Screenshot->setEnabled(true);
                        ui->menuFit_Window->setEnabled(true);
                        ui->menuAspect_Ratio->setEnabled(true);
                        ui->action_Frame_Step->setEnabled(true);
                        ui->actionFrame_Back_Step->setEnabled(true);
                        ui->action_Deinterlace->setEnabled(true);
                        ui->action_Motion_Interpolation->setEnabled(true);
                    }
                    else
                    {
                        if(!albumArt)
                        {
                            // put in filler albumArt
                            if(ui->mpvFrame->styleSheet() == QString())
                                ui->mpvFrame->setStyleSheet("background-image:url(:/images/album-art.png);background-repeat:no-repeat;background-position:center;");
                        }
                        ui->action_Hide_Album_Art->setEnabled(true);
                        ui->menuSubtitle_Track->setEnabled(false);
                        ui->menuFont_Si_ze->setEnabled(false);
                        ui->actionShow_Subtitles->setEnabled(false);
                        ui->actionShow_Subtitles->setChecked(false);
                        ui->menuTake_Screenshot->setEnabled(false);
                        ui->menuFit_Window->setEnabled(false);
                        ui->menuAspect_Ratio->setEnabled(false);
                        ui->action_Frame_Step->setEnabled(false);
                        ui->actionFrame_Back_Step->setEnabled(false);
                        ui->action_Deinterlace->setEnabled(false);
                        ui->action_Motion_Interpolation->setEnabled(false);
                    }

                    if(ui->menuAudio_Tracks->actions().count() == 1)
                        ui->menuAudio_Tracks->actions().first()->setEnabled(false);

                    if(pathChanged && autoFit)
                    {
                        sugoi->FitWindow(autoFit, false);
                        pathChanged = false;
                    }
                }
            });

    connect(mpv, &MpvHandler::chaptersChanged,
            [=](const QList<Mpv::Chapter> &chapters)
            {
                if(mpv->getPlayState() > 0)
                {
                    QAction *action;
                    QList<int> ticks;
                    int n = 1,
                        N = chapters.length();
                    ui->menu_Chapters->clear();
                    for(auto &ch : chapters)
                    {
                        action = ui->menu_Chapters->addAction(QString("%0: %1").arg(Util::FormatNumberWithAmpersand(n, N), ch.title));
                        if(n <= 9)
                            action->setShortcut(QKeySequence("Ctrl+"+QString::number(n)));
                        connect(action, &QAction::triggered,
                                [=]
                                {
                                    mpv->Seek(ch.time);
                                });
                        ticks.push_back(ch.time);
                        n++;
                    }
                    if(ui->menu_Chapters->actions().count() == 0)
                    {
                        ui->menu_Chapters->setEnabled(false);
                        ui->action_Next_Chapter->setEnabled(false);
                        ui->action_Previous_Chapter->setEnabled(false);
                    }
                    else
                    {
                        ui->menu_Chapters->setEnabled(true);
                        ui->action_Next_Chapter->setEnabled(true);
                        ui->action_Previous_Chapter->setEnabled(true);
                    }

                    ui->seekBar->setTicks(ticks);
                }
            });

    connect(mpv, &MpvHandler::playStateChanged,
            [=](Mpv::PlayState playState)
            {
                switch(playState)
                {
                case Mpv::Loaded:
                    sugoi->mpv->ShowText("Loading...", 0);
                    break;

                case Mpv::Started:
                    if(!init) // will only happen the first time a file is loaded.
                    {
                        ui->action_Play->setEnabled(true);
                        ui->playButton->setEnabled(true);
#if defined(Q_OS_WIN)
                        playpause_toolbutton->setEnabled(true);
#endif
                        //ui->playlistButton->setEnabled(true);
                        ui->action_Show_Playlist->setEnabled(true);
                        ui->menuAudio_Tracks->setEnabled(true);
                        init = true;
                    }
                    SetPlaybackControls(true);
                    mpv->Play();
                    sugoi->overlay->showStatusText(QString(), 0);
                case Mpv::Playing:
                    SetPlayButtonIcon(false);
                    if(onTop == "playing")
                        Util::SetAlwaysOnTop(this, true);
                    if (logo->isVisible())
                        logo->hide();
                    break;

                case Mpv::Paused:
                case Mpv::Stopped:
                    SetPlayButtonIcon(true);
                    if(onTop == "playing")
                        Util::SetAlwaysOnTop(this, false);
                    break;

                case Mpv::Idle:
                    if(init)
                    {
                        if(ui->action_This_File->isChecked()) // repeat this file
                        {
                            if (isVisible() && windowState() != Qt::WindowMinimized)
                            {
                                ui->playlistWidget->PlayIndex(0, true); // restart file
                            }
                        }
                        else if(ui->actionStop_after_Current->isChecked() ||  // stop after playing this file
                                ui->playlistWidget->CurrentIndex() >= ui->playlistWidget->count()-1) // end of the playlist
                        {
                            if(!ui->actionStop_after_Current->isChecked() && // not supposed to stop after current
                                ui->action_Playlist->isChecked() && // we're supposed to restart the playlist
                                ui->playlistWidget->count() > 0) // playlist isn't empty
                            {
                                if (isVisible() && windowState() != Qt::WindowMinimized)
                                {
                                    ui->playlistWidget->PlayIndex(0); // restart playlist
                                }
                            }
                            else // stop
                            {
                                setWindowTitle("Sugoi Player");
                                SetPlaybackControls(false);
                                ui->seekBar->setTracking(0);
                                ui->actionStop_after_Current->setChecked(false);
                                if(ui->mpvFrame->styleSheet() != QString()) // remove filler album art
                                    ui->mpvFrame->setStyleSheet("");
                                if (logo->isHidden())
                                    logo->show();
                            }
                        }
                        else
                        {
                            if (isVisible() && windowState() != Qt::WindowMinimized)
                            {
                                ui->playlistWidget->PlayIndex(1, true);
                            }
                        }
                    }
                    break;
                }
            });

    connect(mpv, &MpvHandler::pathChanged,
            [=](QString path)
            {
                pathChanged = true;
                setLastDir(QDir::toNativeSeparators(path));
            });

    connect(mpv, &MpvHandler::fileChanging,
              [=](int t, int l)
              {
                  if(current != nullptr)
                  {
                      if(t > 0.05*l && t < 0.95*l) // only save if within the middle 90%
                          current->time = t;
                      else
                          current->time = 0;
                  }
              });

    connect(mpv, &MpvHandler::timeChanged,
            [=](int i)
            {
                const Mpv::FileInfo &fi = mpv->getFileInfo();
                // set the seekBar's location with NoSignal function so that it doesn't trigger a seek
                // the formula is a simple ratio seekBar's max * time/totalTime
                double currentPercent = (double)i/fi.length;
                currentPercent = currentPercent >= 0.0 ? currentPercent : 0.0;
                ui->seekBar->setValueNoSignal(ui->seekBar->maximum()*currentPercent);

                taskbarProgress->setValue(taskbarProgress->maximum()*currentPercent);

                if (fullscreenProgressIndicator)
                {
                    fullscreenProgressIndicator->setValue(fullscreenProgressIndicator->maximum()*currentPercent);
                }

                SetRemainingLabels(i);

                // set next/previous chapter's enabled state
                if(fi.chapters.length() > 0)
                {
                    ui->action_Next_Chapter->setEnabled(i < fi.chapters.last().time);
                    ui->action_Previous_Chapter->setEnabled(i > fi.chapters.first().time);
                }
            });

    connect(mpv, &MpvHandler::volumeChanged, ui->volumeSlider, &CustomSlider::setValueNoSignal);

    connect(mpv, &MpvHandler::speedChanged,
            [=](double speed)
            {
                static double last = 1;
                if(last != speed)
                {
                    if(init)
                        mpv->ShowText(tr("Speed: %0x").arg(QString::number(speed)));
                    if(speed <= 0.25)
                        ui->action_Decrease->setEnabled(false);
                    else
                        ui->action_Decrease->setEnabled(true);
                    last = speed;
                }
            });

    connect(mpv, &MpvHandler::sidChanged,
            [=](int sid)
            {
                QList<QAction*> actions = ui->menuSubtitle_Track->actions();
                for(auto &action : actions)
                {
                    if(action->text().startsWith(QString::number(sid)))
                    {
                        action->setCheckable(true);
                        action->setChecked(true);
                    }
                    else
                        action->setChecked(false);
                }
            });

    connect(mpv, &MpvHandler::aidChanged,
            [=](int aid)
            {
                QList<QAction*> actions = ui->menuAudio_Tracks->actions();
                for(auto &action : actions)
                {
                    if(action->text().startsWith(QString::number(aid)))
                    {
                        action->setCheckable(true);
                        action->setChecked(true);
                    }
                    else
                        action->setChecked(false);
                }
            });

    connect(mpv, &MpvHandler::subtitleVisibilityChanged,
            [=](bool b)
            {
                if(ui->actionShow_Subtitles->isEnabled())
                    ui->actionShow_Subtitles->setChecked(b);
                if(init)
                    mpv->ShowText(b ? tr("Subtitles visible") : tr("Subtitles hidden"));
            });

    connect(mpv, &MpvHandler::muteChanged,
            [=](bool b)
            {
                if(b)
                    ui->muteButton->setIcon(QIcon(":/images/default_mute.svg"));
                else
                    ui->muteButton->setIcon(QIcon(":/images/default_unmute.svg"));
                mpv->ShowText(b ? tr("Muted") : tr("Unmuted"));
            });

    connect(mpv, &MpvHandler::voChanged,
            [=](QString vo)
            {
                ui->action_Motion_Interpolation->setChecked(vo.contains("interpolation"));
    });
}

void MainWindow::disconnectMpvSignalsAndSlots()
{
    mpv->disconnect();
}

void MainWindow::reconnectMpvSignalsAndSlots()
{
    disconnectMpvSignalsAndSlots();
    connectMpvSignalsAndSlots();
}

void MainWindow::connectUiSignalsAndSlots()
{
    connect(ui->playlistButton, &QPushButton::clicked, this, &MainWindow::TogglePlaylist);

    connect(ui->seekBar, &SeekBar::valueChanged,                        // Playback: Seekbar clicked
            [=](int i)
            {
                mpv->Seek(mpv->Relative(((double)i/ui->seekBar->maximum())*mpv->getFileInfo().length), true);
            });

    connect(ui->openButton, &OpenButton::LeftClick, sugoi, &SugoiEngine::Open);
    connect(ui->openButton, &OpenButton::MiddleClick, sugoi, &SugoiEngine::Jump);
    connect(ui->openButton, &OpenButton::RightClick, sugoi, &SugoiEngine::OpenLocation);

    connect(ui->rewindButton, &QPushButton::clicked,                    // Playback: Rewind button
            [=]
            {
                mpv->Rewind();
            });

    connect(ui->previousButton, &IndexButton::clicked,                  // Playback: Previous button
            [=]
            {
                ui->playlistWidget->PlayIndex(-1, true);
            });

    connect(ui->playButton, &QPushButton::clicked, sugoi, &SugoiEngine::PlayPause);

    connect(ui->nextButton, &IndexButton::clicked,                      // Playback: Next button
            [=]
            {
                ui->playlistWidget->PlayIndex(1, true);
            });

    connect(ui->muteButton, &QPushButton::clicked,
            [=]
            {
                mpv->Mute(!mpv->getMute());
            });

    connect(ui->volumeSlider, &CustomSlider::valueChanged,              // Playback: Volume slider adjusted
            [=](int i)
            {
                mpv->Volume(i, true);
            });

    connect(ui->splitter, &CustomSplitter::positionChanged,             // Splitter position changed
            [=](int i)
            {
                blockSignals(true);
                if(i == 0) // right-most, playlist is hidden
                {
                    ui->action_Show_Playlist->setChecked(false);
                    ui->action_Hide_Album_Art->setChecked(false);
                    ui->playlistLayoutWidget->setVisible(false);
                }
                else if(i == ui->splitter->max()) // left-most, album art is hidden, playlist is visible
                {
                    ui->action_Show_Playlist->setChecked(true);
                    ui->action_Hide_Album_Art->setChecked(true);
                }
                else // in the middle, album art is visible, playlist is visible
                {
                    ui->action_Show_Playlist->setChecked(true);
                    ui->action_Hide_Album_Art->setChecked(false);
                }
                ui->playlistLayoutWidget->setVisible(ui->action_Show_Playlist->isChecked());
                blockSignals(false);
                if(ui->actionMedia_Info->isChecked())
                    sugoi->overlay->showInfoText();

                logo->setGeometry(0, menuBar()->height() / 2, width() - i,
                    height() - menuBar()->height() / 2 - ui->seekBar->height() - ui->playbackLayoutWidget->height());
            });

    connect(ui->searchBox, &QLineEdit::textChanged,                     // Playlist: Search box
            [=](QString s)
            {
                ui->playlistWidget->Search(s);
            });

    connect(ui->indexLabel, &CustomLabel::clicked,                      // Playlist: Clicked the indexLabel
            [=]
            {
                QString res = InputDialog::getInput(tr("Enter the file number you want to play:\nNote: Value must be from %0 - %1").arg("1", QString::number(ui->playlistWidget->count())),
                                                    tr("Enter File Number"),
                                                    [this](QString input)
                                                    {
                                                        int in = input.toInt();
                                                        if(in >= 1 && in <= ui->playlistWidget->count())
                                                            return true;
                                                        return false;
                                                    },
                                                    this);
                if(res != "")
                    ui->playlistWidget->PlayIndex(res.toInt()-1); // user index will be 1 greater than actual
            });

    connect(ui->playlistWidget, &PlaylistWidget::currentRowChanged,     // Playlist: Playlist selection changed
            [=](int)
            {
                SetIndexLabels(true);
            });

    connect(ui->currentFileButton, &QPushButton::clicked,               // Playlist: Select current file button
            [=]
            {
                ui->playlistWidget->SelectIndex(ui->playlistWidget->CurrentIndex());
            });

    connect(ui->refreshButton, &QPushButton::clicked,                   // Playlist: Refresh playlist button
            [=]
            {
                ui->playlistWidget->RefreshPlaylist();
            });

    connect(ui->inputLineEdit, &CustomLineEdit::submitted,
            [=](QString s)
            {
                sugoi->Command(s);
                ui->inputLineEdit->setText("");
    });
}

void MainWindow::disconnectUiSignalsAndSlots()
{
    ui->playlistButton->disconnect();
    ui->seekBar->disconnect();
    ui->openButton->disconnect();
    ui->rewindButton->disconnect();
    ui->previousButton->disconnect();
    ui->playButton->disconnect();
    ui->nextButton->disconnect();
    ui->muteButton->disconnect();
    ui->volumeSlider->disconnect();
    ui->splitter->disconnect();
    ui->searchBox->disconnect();
    ui->indexLabel->disconnect();
    ui->playlistWidget->disconnect();
    ui->currentFileButton->disconnect();
    ui->refreshButton->disconnect();
    ui->inputLineEdit->disconnect();
}

void MainWindow::reconnectUiSignalsAndSlots()
{
    disconnectUiSignalsAndSlots();
    connectUiSignalsAndSlots();
}

void MainWindow::connectOtherSignalsAndSlots()
{
    // command action mappings (action (right) performs command (left))
    commandActionMap = {
        {"mpv add chapter +1", ui->action_Next_Chapter},
        {"mpv add chapter -1", ui->action_Previous_Chapter},
        {"mpv set sub-scale 1", ui->action_Reset_Size},
        {"mpv add sub-scale +0.1", ui->action_Size},
        {"mpv add sub-scale -0.1", ui->actionS_ize},
        {"mpv set video-aspect -1", ui->action_Auto_Detect}, // todo: make these sugoi-commands so we can output messages when they change
        {"mpv set video-aspect 16:9", ui->actionForce_16_9},
        {"mpv set video-aspect 2.35:1", ui->actionForce_2_35_1},
        {"mpv set video-aspect 4:3", ui->actionForce_4_3},
        {"mpv cycle sub-visibility", ui->actionShow_Subtitles},
        {"mpv set time-pos 0", ui->action_Restart},
        {"mpv frame_step", ui->action_Frame_Step},
        {"mpv frame_back_step", ui->actionFrame_Back_Step},
        {"deinterlace", ui->action_Deinterlace},
        {"interpolate", ui->action_Motion_Interpolation},
        {"mute", ui->action_Mute},
        {"screenshot subtitles", ui->actionWith_Subtitles},
        {"screenshot", ui->actionWithout_Subtitles},
        {"add_subtitles", ui->action_Add_Subtitle_File},
        {"add_audio", ui->action_Add_Audio_File},
        {"fitwindow", ui->action_To_Current_Size},
        {"fitwindow 50", ui->action50},
        {"fitwindow 75", ui->action75},
        {"fitwindow 100", ui->action100},
        {"fitwindow 150", ui->action150},
        {"fitwindow 200", ui->action200},
        {"fullscreen", ui->action_Full_Screen},
        {"hide_all_controls", ui->actionHide_All_Controls},
        {"jump", ui->action_Jump_to_Time},
        {"media_info", ui->actionMedia_Info},
        {"new", ui->action_New_Player},
        {"open", ui->action_Open_File},
        {"open_clipboard", ui->actionOpen_Path_from_Clipboard},
        {"open_location", ui->actionOpen_URL},
        {"playlist play +1", ui->actionPlay_Next_File},
        {"playlist play -1", ui->actionPlay_Previous_File},
        {"playlist repeat off", ui->action_Off},
        {"playlist repeat playlist", ui->action_Playlist},
        {"playlist repeat this", ui->action_This_File},
        {"playlist shuffle", ui->actionSh_uffle},
        {"playlist toggle", ui->action_Show_Playlist},
        {"playlist full", ui->action_Hide_Album_Art},
        {"dim", ui->action_Dim_Lights},
        {"play_pause", ui->action_Play},
        {"quit", ui->actionE_xit},
        {"show_in_folder", ui->actionShow_in_Folder},
        {"stop", ui->action_Stop},
        {"volume +5", ui->action_Increase_Volume},
        {"volume -5", ui->action_Decrease_Volume},
        {"speed +0.1", ui->action_Increase},
        {"speed -0.1", ui->action_Decrease},
        {"speed 1.0", ui->action_Reset},
        {"output", ui->actionShow_D_ebug_Output},
        {"preferences", ui->action_Preferences},
        {"online_help", ui->actionOnline_Help},
        {"bug_report", ui->action_Report_bugs},
        {"sys_info", ui->action_System_Information},
        {"update", ui->action_Check_for_Updates},
        {"update youtube-dl", ui->actionUpdate_Streaming_Support},
        {"about", ui->actionAbout_Sugoi_Player}
    };

    // map actions to commands
    for(auto action = commandActionMap.begin(); action != commandActionMap.end(); ++action)
    {
        const QString cmd = action.key();
        connect(*action, &QAction::triggered,
                [=] { sugoi->Command(cmd); });
    }

    connect(this, &MainWindow::skinFileChanged,
            [=](const QString &skin)
            {
                if (skin.isEmpty())
                {
                    return;
                }
                SkinManager::instance()->setSkin(skin);
            });

    connect(this, &MainWindow::autoUpdatePlayerChanged,
            [=](bool isAuto)
            {
#ifdef _DEBUG
                return;
#endif
                if (isAuto)
                {
                    win_sparkle_set_automatic_check_for_updates(1);
                }
                else
                {
                    win_sparkle_set_automatic_check_for_updates(0);
                }
            });

    connect(this, &MainWindow::osdShowLocalTimeChanged,
            [=](bool enable)
            {
                if (osdLocalTimeUpdater)
                {
                    if (enable)
                    {
                        if (osdLocalTimeUpdater->isActive())
                        {
                            osdLocalTimeUpdater->stop();
                        }
                        osdLocalTimeUpdater->start(1000);
                    }
                    else
                    {
                        osdLocalTimeUpdater->stop();
                    }
                }
            });

    connect(this, &MainWindow::showFullscreenIndicatorChanged,
            [=](bool show)
            {
                if (show)
                {
                    if (isFullScreenMode())
                    {
                        if (fullscreenProgressIndicator == nullptr)
                        {
                            fullscreenProgressIndicator = new ProgressIndicatorBar(this);
                            fullscreenProgressIndicator->setFixedSize(QSize(width(), (2.0 / 1080.0) * height()));
                            fullscreenProgressIndicator->move(QPoint(0, height() - fullscreenProgressIndicator->height()));
                            fullscreenProgressIndicator->setRange(0, 1000);
                        }
                        fullscreenProgressIndicator->show();
                    }
                }
                else
                {
                    if (fullscreenProgressIndicator)
                    {
                        fullscreenProgressIndicator->close();
                        delete fullscreenProgressIndicator;
                        fullscreenProgressIndicator = nullptr;
                    }
                }
            });

    connect(this, &MainWindow::langChanged,
            [=](QString l)
            {
                lang = QString::fromLatin1("auto");
                l = l.replace('-', '_');
                if (l == QString::fromLatin1("auto") || l == QString::fromLatin1("system") || l == QString::fromLatin1("ui") || l == QString::fromLatin1("locale"))
                {
                    QLocale locale;
                    l = locale.uiLanguages().first();
                    l = l.replace('-', '_');
                }

                if (l != QString::fromLatin1("C") && l != QString::fromLatin1("en") && l != QString::fromLatin1("en_US") && l != QString::fromLatin1("en_UK"))
                {
                    // load the application translations
                    if(sugoi->translator != nullptr)
                    {
                        qApp->removeTranslator(sugoi->translator);
                        delete sugoi->translator;
                        sugoi->translator = nullptr;
                    }
                    sugoi->translator = new QTranslator(qApp);
                    QString langPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
                    if (sugoi->translator->load(QString("sugoi_%0").arg(l), langPath))
                    {
                        if (qApp->installTranslator(sugoi->translator))
                        {
                            lang = l;
                        }
                    }
                    else
                    {
                        delete sugoi->translator;
                        sugoi->translator = nullptr;
                    }
                }
                else
                {
                    if(sugoi->translator != nullptr)
                    {
                        qApp->removeTranslator(sugoi->translator);
                        delete sugoi->translator;
                        sugoi->translator = nullptr;
                    }
                }

                // save strings we want to keep
                QString title = windowTitle(),
                        duration = ui->durationLabel->text(),
                        remaining = ui->remainingLabel->text(),
                        index = ui->indexLabel->text();

                ui->retranslateUi(this);

                // reload strings we kept
                setWindowTitle(title);
                ui->durationLabel->setText(duration);
                ui->remainingLabel->setText(remaining);
                ui->indexLabel->setText(index);
            });

    connect(this, &MainWindow::debugChanged,
            [=](bool b)
            {
                ui->actionShow_D_ebug_Output->setChecked(b);
                ui->verticalWidget->setVisible(b);
                mouseMoveEvent(new QMouseEvent(QMouseEvent::MouseMove,
                                               QCursor::pos(),
                                               Qt::NoButton,Qt::NoButton,Qt::NoModifier));
                if(b)
                    ui->inputLineEdit->setFocus();
            });

    connect(this, &MainWindow::hideAllControlsChanged,
            [=](bool b)
            {
                HideAllControls(b);
                blockSignals(true);
                ui->actionHide_All_Controls->setChecked(b);
                blockSignals(false);
            });

    connect(sugoi->sysTrayIcon, &QSystemTrayIcon::activated, this, &MainWindow::BringWindowToFront);

    connect(sugoi->sysTrayIcon, &QSystemTrayIcon::messageClicked, this, &MainWindow::BringWindowToFront);

    connect(this, &MainWindow::trayIconVisibleChanged,
            [=](bool visible)
            {
                if (sugoi->sysTrayIcon != nullptr)
                {
                    sugoi->sysTrayIcon->setVisible(visible);
                }
            });

    connect(this, &MainWindow::quickStartModeChanged,
            [=](bool quickStart)
            {
                if (this->isHidden())
                {
                    return;
                }
                if (quickStart)
                {
                    if (!Util::isAutoStart())
                    {
                        const QString exePath = QCoreApplication::applicationFilePath();
                        const QString exeParam = QString::fromLatin1("--autostart");
                        Util::executeProgramWithAdministratorPrivilege(exePath, exeParam);
                    }
                }
                else
                {
                    if (Util::isAutoStart())
                    {
                        const QString exePath = QCoreApplication::applicationFilePath();
                        const QString exeParam = QString::fromLatin1("--noautostart");
                        Util::executeProgramWithAdministratorPrivilege(exePath, exeParam);
                    }
                }
            });

    connect(this, &MainWindow::openFileFromCmd,
            [=](const QString &filePath)
            {
                if (!filePath.isEmpty())
                {
                    if (closing)
                    {
                        QTimer::singleShot(2000, [=]
                        {
                            QtConcurrent::run([=]
                            {
                                mpv->LoadFile(filePath);
                            });
                        });
                    }
                    else
                    {
                        QtConcurrent::run([=]
                        {
                            mpv->LoadFile(filePath);
                        });
                    }
                }
            });

    connect(this, &MainWindow::onTopChanged,
            [=](QString onTop)
            {
                if(onTop == "never")
                    Util::SetAlwaysOnTop(this, false);
                else if(onTop == "always")
                    Util::SetAlwaysOnTop(this, true);
                else if(onTop == "playing" && mpv->getPlayState() > 0)
                    Util::SetAlwaysOnTop(this, true);
            });

    connect(this, &MainWindow::remainingChanged,
            [=]
            {
                SetRemainingLabels(mpv->getTime());
            });

    connect(autohide, &QTimer::timeout, // cursor autohide
            [=]
            {
                if(ui->mpvFrame->geometry().contains(ui->mpvFrame->mapFromGlobal(cursor().pos())))
                    setCursor(QCursor(Qt::BlankCursor));
                if(autohide)
                    autohide->stop();
            });

    connect(osdLocalTimeUpdater, &QTimer::timeout,
            [=]
            {
                if (osdShowLocalTime)
                {
                    if (mpv->getPlayState() > 0)
                    {
                        QString curTime = QTime::currentTime().toString("hh:mm:ss");
                        if (!curTime.isEmpty())
                        {
                            mpv->ShowText(curTime, 1500);
                        }
                    }
                }
                else
                {
                    osdLocalTimeUpdater->stop();
                }
            });

    // dimDialog
    connect(sugoi->dimDialog, &DimDialog::visbilityChanged,
            [=](bool dim)
            {
                ui->action_Dim_Lights->setChecked(dim);
                if(dim)
                    Util::SetAlwaysOnTop(this, true);
                else if(onTop == "never" || (onTop == "playing" && mpv->getPlayState() > 0))
                    Util::SetAlwaysOnTop(this, false);
            });

    connect(this, &MainWindow::playlistChanged,
            [=](const QStringList &list)
            {
                if(list.length() > 1)
                {
                    ui->actionSh_uffle->setEnabled(true);
                    ui->actionStop_after_Current->setEnabled(true);
                    //ShowPlaylist(true);
                }
                else
                {
                    ui->actionSh_uffle->setEnabled(false);
                    ui->actionStop_after_Current->setEnabled(false);
                }

                if(list.length() > 0)
                    ui->menuR_epeat->setEnabled(true);
                else
                    ui->menuR_epeat->setEnabled(false);
            });
}

void MainWindow::disconnectOtherSignalsAndSlots()
{
    for(auto action = commandActionMap.begin(); action != commandActionMap.end(); ++action)
    {
        static_cast<QObject *>(*action)->disconnect();
    }
    sugoi->sysTrayIcon->disconnect();
    autohide->disconnect();
    osdLocalTimeUpdater->disconnect();
    sugoi->dimDialog->disconnect();
    this->disconnect();
}

void MainWindow::reconnectOtherSignalsAndSlots()
{
    disconnectOtherSignalsAndSlots();
    connectOtherSignalsAndSlots();
}

void MainWindow::reconnectAllSignalsAndSlots()
{
    reconnectMpvSignalsAndSlots();
    reconnectUiSignalsAndSlots();
    reconnectOtherSignalsAndSlots();
}
