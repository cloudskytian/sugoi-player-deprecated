#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : CFramelessWindow(parent)
  , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setMinimumSize(size());

    setTitleBar(ui->titleBarWidget);
    addIgnoreWidget(ui->windowTitleLabel);
    setContentsMargins(0, 0, 0, 0);

#if defined(Q_OS_UNIX) || defined(Q_OS_LINUX)
    // update streaming support disabled on unix platforms
    ui->actionUpdate_Streaming_Support->setEnabled(false);
#endif

    ShowPlaylist(false);
    addActions(ui->menuBarWidget->actions()); // makes menubar shortcuts work even when menubar is hidden

    // initialize managers/handlers
    sugoi = new SugoiEngine(this);
    mpv = ui->mpvFrame;
    mpv->SetEngine(sugoi);

    ui->playlistWidget->AttachEngine(sugoi);
    ui->playbackLayoutWidget->installEventFilter(this);
    ui->mpvFrame->installEventFilter(this); // capture events on mpvFrame in the eventFilter function
    autohide = new QTimer(this);
    osdLocalTimeUpdater = new QTimer(this);
}

MainWindow::~MainWindow()
{
#ifdef Q_OS_WIN
    delete prev_toolbutton;
    delete playpause_toolbutton;
    delete next_toolbutton;
    delete thumbnail_toolbar;
    delete taskbarProgress;
    delete taskbarButton;
    delete jumplist;
#endif
    delete ui;
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
    CFramelessWindow::dragEnterEvent(event);
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
    CFramelessWindow::dropEvent(event);
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        bool shouldMove = false;
        if (ui->remainingLabel->rect().contains(ui->remainingLabel->mapFrom(this, event->pos()))) // clicked timeLayoutWidget
        {
            setRemaining(!remaining); // todo: use a sugoicommand
        }
        else if (ui->mpvFrame->geometry().contains(event->pos())
                 && !ui->playbackLayoutWidget->geometry().contains(event->pos())) // mouse is in the mpvFrame
        {
            mpv->PlayPause(ui->playlistWidget->CurrentItem());
            shouldMove = true;
        }
        else if (!ui->seekBar->geometry().contains(event->pos()))
        {
            shouldMove = true;
        }
        if (shouldMove && !isFullScreen() && !isMaximized())
        {
#ifdef Q_OS_WIN
            if (ReleaseCapture())
            {
                SendMessage(HWND(winId()), WM_SYSCOMMAND, SC_MOVE + HTCAPTION, 0);
            }
            event->ignore();
#else
            //TODO: Other platforms
#endif
        }
    }
    CFramelessWindow::mousePressEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (isFullScreenMode())
    {
        setCursor(QCursor(Qt::ArrowCursor)); // show the cursor
        autohide->stop();

        QRect playbackRect = geometry();
        playbackRect.setTop(playbackRect.bottom() - 60);
        bool showPlayback = playbackRect.contains(event->globalPos());
        bool showBottomControlPanel = showPlayback || ui->outputTextEdit->isVisible();

        if (showBottomControlPanel)
        {
            if (fullscreenProgressIndicator)
            {
                fullscreenProgressIndicator->hide();
            }
            ui->playbackLayoutWidget->show();
            ui->seekBar->show();
        }
        else
        {
            ui->seekBar->hide();
            ui->playbackLayoutWidget->hide();
            if (fullscreenProgressIndicator)
            {
                fullscreenProgressIndicator->show();
            }
        }

        QRect playlistRect = geometry();
        playlistRect.setLeft(playlistRect.right() - qCeil(playlistRect.width()/7.0));
        bool showPlaylist = playlistRect.contains(event->globalPos());
        ShowPlaylist(showPlaylist);

        if (!(showPlayback || showPlaylist) && autohide)
        {
            autohide->start(500);
        }
    }
    CFramelessWindow::mouseMoveEvent(event);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if((obj == ui->mpvFrame || obj == ui->playbackLayoutWidget)
            && isFullScreenMode() && event->type() == QEvent::MouseMove)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        mouseMoveEvent(mouseEvent);
        return true;
    }
    else if(event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        keyPressEvent(keyEvent);
        return true;
    }
    return false;
}

void MainWindow::wheelEvent(QWheelEvent *event)
{
    if(event->delta() > 0)
        mpv->Volume(mpv->getVolume()+5, true);
    else
        mpv->Volume(mpv->getVolume()-5, true);
    CFramelessWindow::wheelEvent(event);
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
    CFramelessWindow::keyPressEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    if(ui->actionMedia_Info->isChecked())
    {
        sugoi->overlay->showInfoText();
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

    CFramelessWindow::resizeEvent(event);
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange)
    {
        if (windowState() == Qt::WindowMinimized)
        {
            if (pauseWhenMinimized)
            {
                if (IsPlayingVideo(mpv->getFile()))
                {
                    mpv->Pause();
                }
            }
        }
        else if (windowState() == Qt::WindowMaximized)
        {
            ui->maximizeButton->setIcon(QIcon(":/images/disabled_restore.svg"));
        }
        else if (windowState() == Qt::WindowNoState)
        {
            ui->maximizeButton->setIcon(QIcon(":/images/disabled_maximize.svg"));
        }
    }
    CFramelessWindow::changeEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    sugoi->SaveSettings();
    bool canClose = true;
    if (IsPlayingMusic(mpv->getFile()))
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
        playInBackground = true;
        hide();
        if (sugoi->sysTrayIcon->isVisible())
        {
            sugoi->sysTrayIcon->showMessage(QString::fromLatin1("Sugoi Player"), tr("Sugoi Player is running in background now, click the trayicon to bring Sugoi Player to foreground."), QIcon(":/images/player.svg"), 4000);
        }
        event->ignore();
        return;
    }
    else
    {
        playInBackground = false;
    }
    event->accept();
    CFramelessWindow::closeEvent(event);
}

void MainWindow::showEvent(QShowEvent *event)
{
    CFramelessWindow::showEvent(event);
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
        win_sparkle_set_appcast_url("https://raw.githubusercontent.com/wangwenx190/Sugoi-Player/master/appcast.xml");
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
    if(event->button() == Qt::LeftButton && ui->mpvFrame->geometry().contains(event->pos())
            && !ui->playbackLayoutWidget->geometry().contains(event->pos())) // if mouse is in the mpvFrame
    {
        if(!isFullScreen() && ui->action_Full_Screen->isEnabled()) // don't allow people to go full screen if they shouldn't be able to
            FullScreen(true);
        else // they can leave fullscreen even if it's disabled (eg. video ends while in full screen)
            FullScreen(false);
        event->accept();
    }
    CFramelessWindow::mouseDoubleClickEvent(event);
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
        ui->menuBarWidget->setVisible(false);

        ui->playbackLayoutWidget->hide();
        ui->seekBar->hide();
        ui->centralWidget->layout()->removeWidget(ui->playbackLayoutWidget);
        ui->playbackLayoutWidget->setParent(this);
        ui->playbackLayoutWidget->move(QPoint(0, geometry().height() - ui->playbackLayoutWidget->height()));
        ui->centralWidget->layout()->removeWidget(ui->seekBar);
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

        ui->seekBar->setParent(ui->centralWidget);
        ui->centralWidget->layout()->addWidget(ui->seekBar);
        ui->playbackLayoutWidget->setParent(ui->centralWidget);
        ui->centralWidget->layout()->addWidget(ui->playbackLayoutWidget);

        if(menuVisible)
            ui->menuBarWidget->setVisible(true);
        ui->seekBar->setVisible(true);
        ui->playbackLayoutWidget->setVisible(true);
        setCursor(QCursor(Qt::ArrowCursor)); // show cursor
        autohide->stop();
        ShowPlaylist(playlistState);
    }
}

void MainWindow::FullScreen(bool fs)
{
    static Qt::WindowStates oldState;
    if (fs)
    {
        if(sugoi->dimDialog && sugoi->dimDialog->isVisible())
            sugoi->Dim(false);
        if (ui->menuBarWidget && ui->menuBarWidget->isVisible())
        {
            ui->menuBarWidget->hide();
        }
        if (ui->titleBarWidget->isVisible())
        {
            ui->titleBarWidget->hide();
        }
        oldState = windowState();
        showFullScreen();
        if(!hideAllControls)
        {
            HideAllControls(true, false);
        }
    }
    else
    {
        setWindowState(oldState);
        if (!ui->titleBarWidget->isVisible())
        {
            ui->titleBarWidget->show();
        }
        if (ui->menuBarWidget && !ui->menuBarWidget->isVisible())
        {
            ui->menuBarWidget->show();
        }
        if(!hideAllControls)
        {
            HideAllControls(false, false);
        }
    }
}

bool MainWindow::isPlaylistVisible()
{
    // if the position is 0, playlist is hidden
    return (ui->splitter->position() != 0);
}

void MainWindow::SetWindowTitle2(const QString &text)
{
    setWindowTitle(text);
    ui->windowTitleLabel->setText(text);
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
    }
    else
    {
        if(ui->splitter->position() != ui->splitter->max() && ui->splitter->position() != 0)
            ui->splitter->setNormalPosition(ui->splitter->position()); // save current splitter position as the normal position
        ui->splitter->setPosition(0); // set splitter position to right-most
        setFocus();
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
#ifdef Q_OS_WIN
    next_toolbutton->setEnabled(enable);
#endif
}

void MainWindow::SetPreviousButtonEnabled(bool enable)
{
    ui->previousButton->setEnabled(enable);
    ui->actionPlay_Previous_File->setEnabled(enable);
#ifdef Q_OS_WIN
    prev_toolbutton->setEnabled(enable);
#endif
}

void MainWindow::SetPlayButtonIcon(bool play)
{
    if(play)
    {
        ui->playButton->setIcon(QIcon(":/images/default_play.svg"));
        ui->action_Play->setText(tr("&Play"));
#ifdef Q_OS_WIN
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
#ifdef Q_OS_WIN
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
    if (fi.length == 0)
    {
        if (ui->remainingLabel->isVisible())
        {
            ui->remainingLabel->hide();
        }
        if (ui->seperatorLabel->isVisible())
        {
            ui->seperatorLabel->hide();
        }

        ui->durationLabel->setText(Util::FormatTime(time, time));
    }
    else
    {
        if (ui->remainingLabel->isHidden())
        {
            ui->remainingLabel->show();
        }
        if (ui->seperatorLabel->isHidden())
        {
            ui->seperatorLabel->show();
        }

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

void MainWindow::initMainWindow(bool backgroundMode)
{
    menuVisible = true; //ui->menuBarWidget->isVisible(); // does the OS use a menubar? (appmenu doesn't)
#ifdef Q_OS_WIN
    QtWin::enableBlurBehindWindow(this);

    jumplist = new QWinJumpList(this);
    jumplist->recent()->setVisible(true);

    taskbarButton = new QWinTaskbarButton(this);
    taskbarButton->setWindow(windowHandle());

    taskbarProgress = taskbarButton->progress();
    taskbarProgress->setMinimum(0);
    taskbarProgress->setMaximum(1000);

    // add windows 7+ thubnail toolbar buttons
    thumbnail_toolbar = new QWinThumbnailToolBar(this);
    thumbnail_toolbar->setWindow(windowHandle());

    prev_toolbutton = new QWinThumbnailToolButton(thumbnail_toolbar);
    prev_toolbutton->setEnabled(false);
    prev_toolbutton->setToolTip(tr("Previous"));
    prev_toolbutton->setIcon(QIcon(":/images/tool-previous.ico"));

    playpause_toolbutton = new QWinThumbnailToolButton(thumbnail_toolbar);
    playpause_toolbutton->setEnabled(false);
    playpause_toolbutton->setToolTip(tr("Play"));
    playpause_toolbutton->setIcon(QIcon(":/images/tool-play.ico"));

    next_toolbutton = new QWinThumbnailToolButton(thumbnail_toolbar);
    next_toolbutton->setEnabled(false);
    next_toolbutton->setToolTip(tr("Next"));
    next_toolbutton->setIcon(QIcon(":/images/tool-next.ico"));

    thumbnail_toolbar->addButton(prev_toolbutton);
    thumbnail_toolbar->addButton(playpause_toolbutton);
    thumbnail_toolbar->addButton(next_toolbutton);

    connect(prev_toolbutton, &QWinThumbnailToolButton::clicked,
            [=]
            {
                ui->playlistWidget->PlayIndex(-1, true);
            });

    connect(playpause_toolbutton, &QWinThumbnailToolButton::clicked,
            [=]
            {
                sugoi->PlayPause();
            });

    connect(next_toolbutton, &QWinThumbnailToolButton::clicked,
            [=]
            {
                ui->playlistWidget->PlayIndex(1, true);
            });
#endif

    // add multimedia shortcuts
    ui->action_Play->setShortcuts({ui->action_Play->shortcut(), QKeySequence(Qt::Key_MediaPlay)});
    ui->action_Stop->setShortcuts({ui->action_Stop->shortcut(), QKeySequence(Qt::Key_MediaStop)});
    ui->actionPlay_Next_File->setShortcuts({ui->actionPlay_Next_File->shortcut(), QKeySequence(Qt::Key_MediaNext)});
    ui->actionPlay_Previous_File->setShortcuts({ui->actionPlay_Previous_File->shortcut(), QKeySequence(Qt::Key_MediaPrevious)});

    sugoi->LoadSettings();

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
}
