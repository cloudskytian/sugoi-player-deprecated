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

    addActions(ui->menuBarWidget->actions());
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

void MainWindow::setWindowTitle2(const QString &text)
{
    setWindowTitle(text);
    ui->windowTitleLabel->setText(text);
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
