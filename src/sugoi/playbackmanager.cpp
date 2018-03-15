#include "playbackmanager.h"

#include <QtMath>
#include <QLibraryInfo>
#include <QMimeData>
#include <QMimeDatabase>
#include <QDesktopWidget>
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>
#include <QMessageBox>
#include <QTime>
#include <QUrl>
#include <QCursor>
#include <QtConcurrent>
#include <QApplication>
#include <QTranslator>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QSystemTrayIcon>

#ifdef Q_OS_WIN
#include <WinUser.h>
#include <QtWinExtras>
#include <QtWin>
#include <QWinThumbnailToolBar>
#include <QWinThumbnailToolButton>
#include <QWinTaskbarButton>
#include <QWinTaskbarProgress>
#include <QWinJumpList>
#endif

#include "util.h"
#include "ui/propertieswindow.h"
#include "ui/mainwindow.h"
#include "widgets/mpvwidget.h"
#include "widgets/progressindicatorbar.h"
#include "overlayhandler.h"
#include "widgets/dimdialog.h"
#include "ui/inputdialog.h"
#include "ui/screenshotdialog.h"
#include "winsparkle.h"
#include "skinmanager.h"
#include "widgets/seekbar.h"
#include "widgets/openbutton.h"
#include "widgets/indexbutton.h"
#include "widgets/customlabel.h"
#include "widgets/customlineedit.h"
#include "widgets/customslider.h"
#include "widgets/customsplitter.h"
#include "widgets/playlistwidget.h"

PlaybackManager *PlaybackManager::instance()
{
    static PlaybackManager playbackManager;
    return &playbackManager;
}

PlaybackManager::PlaybackManager(QObject *parent) : QObject(parent)
{
    m_pMainWindow = new MainWindow(nullptr);
#ifndef _DEBUG
    SetParent(reinterpret_cast<HWND>(m_pMainWindow->winId()), GetDesktopWindow());
#endif
    m_pMpvObject = new MpvObject(m_pMainWindow);
    m_pPropertiesWindow = new PropertiesWindow(m_pMainWindow);

    showPlaylist(false);

    sugoi = new SugoiEngine(this);

    m_pMainWindow->ui->playlistWidget->AttachEngine(sugoi);
    m_pMainWindow->ui->playbackLayoutWidget->installEventFilter(this);
    m_pMainWindow->ui->mpvFrame->installEventFilter(this);
    osdLocalTimeUpdater = new QTimer(this);

    //connect(m_pMainWindow, &MainWindow::destroyed, qApp, &QCoreApplication::aboutToQuit);

    connect(m_pMpvObject, &MpvObject::fileNameChanged, m_pPropertiesWindow, &PropertiesWindow::setFileName);
    connect(m_pMpvObject, &MpvObject::fileFormatChanged, m_pPropertiesWindow, &PropertiesWindow::setFileFormat);
    connect(m_pMpvObject, &MpvObject::fileSizeChanged, m_pPropertiesWindow, &PropertiesWindow::setFileSize);
    connect(m_pMpvObject, &MpvObject::playLengthChanged, m_pPropertiesWindow, &PropertiesWindow::setMediaLength);
    connect(m_pMpvObject, &MpvObject::videoSizeChanged, m_pPropertiesWindow, &PropertiesWindow::setVideoSize);
    connect(m_pMpvObject, &MpvObject::fileCreationTimeChanged, m_pPropertiesWindow, &PropertiesWindow::setFileCreationTime);
    connect(m_pMpvObject, &MpvObject::tracksChanged, m_pPropertiesWindow, &PropertiesWindow::setTracks);
    connect(m_pMpvObject, &MpvObject::mediaTitleChanged, m_pPropertiesWindow, &PropertiesWindow::setMediaTitle);
    connect(m_pMpvObject, &MpvObject::filePathChanged, m_pPropertiesWindow, &PropertiesWindow::setFilePath);
    connect(m_pMpvObject, &MpvObject::metaDataChanged, m_pPropertiesWindow, &PropertiesWindow::setMetaData);
    connect(m_pMpvObject, &MpvObject::chaptersChanged, m_pPropertiesWindow, &PropertiesWindow::setChapters);

    connectMainWindowOtherSignalsAndSlots();
    connectMpvSignalsAndSlots();
    connectMainWindowUiSignalsAndSlots();
}

PlaybackManager::~PlaybackManager()
{
    win_sparkle_cleanup();

    if (currentRecent != nullptr)
    {
        int t = m_pMpvObject->time();
        int l = m_pMpvObject->fileInfo().duration;
        if (t > 0.05 * l && t < 0.95 * l)
        {
            currentRecent->time = t;
        }
        else
        {
            currentRecent->time = 0;
        }
    }

    sugoi->saveSettings();

    if (m_pPropertiesWindow != nullptr)
    {
        delete m_pPropertiesWindow;
        m_pPropertiesWindow = nullptr;
    }
    if (m_pMpvObject != nullptr)
    {
        delete m_pMpvObject;
        m_pMpvObject = nullptr;
    }
    if (m_pMainWindow != nullptr)
    {
        delete m_pMainWindow;
        m_pMainWindow = nullptr;
    }
}

void PlaybackManager::initMainWindow(bool backgroundMode)
{
    if (m_pMainWindow == nullptr)
    {
        return;
    }
    m_pMainWindow->initMainWindow(backgroundMode);
    if (backgroundMode)
    {
        m_pMainWindow->setWindowOpacity(0.0);
        if (sugoi != nullptr)
        {
            if (sugoi->sysTrayIcon != nullptr)
            {
                sugoi->sysTrayIcon->hide();
            }
        }
        m_pMainWindow->hide();
    }
    else
    {
        m_pMainWindow->show();
    }
}

void PlaybackManager::showMainWindow()
{
    if (m_pMainWindow == nullptr)
    {
        return;
    }
    m_pMainWindow->show();
}

void PlaybackManager::hideMainWindow()
{
    if (m_pMainWindow == nullptr)
    {
        return;
    }
    m_pMainWindow->hide();
}

void PlaybackManager::closeMainWindow()
{
    if (m_pMainWindow == nullptr)
    {
        return;
    }
    m_pMainWindow->close();
}

void PlaybackManager::activateMainWindow()
{
    if (m_pMainWindow->isActiveWindow())
    {
        return;
    }
    if (m_pMainWindow->isHidden())
    {
        m_pMainWindow->show();
    }
    if (m_pMainWindow->windowOpacity() < 1.0)
    {
        m_pMainWindow->setWindowOpacity(1.0);
    }
    m_pMainWindow->setWindowState(m_pMainWindow->windowState() & ~Qt::WindowMinimized);
    if (m_pMainWindow->isActiveWindow())
    {
        return;
    }
    Qt::WindowFlags oldFlags = m_pMainWindow->windowFlags();
    m_pMainWindow->setWindowFlags(oldFlags | Qt::WindowStaysOnTopHint);
    m_pMainWindow->show();
    m_pMainWindow->setWindowFlags(oldFlags);
    m_pMainWindow->show();
    if (m_pMainWindow->isActiveWindow())
    {
        return;
    }
    m_pMainWindow->raise();
    m_pMainWindow->activateWindow();
}

void PlaybackManager::setMediaFileAssociations(FileAssoc::reg_type type, bool showUI)
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
            currentRegType = type;
            FileAssoc fileAssoc;
            currentRegState = fileAssoc.getMediaFilesRegisterState();
        }
    }
}

void PlaybackManager::hideAllControls(bool w, bool s)
{
    if (s)
    {
        hideAllControls = w;
        if (m_pMainWindow->isFullScreen())
        {
            return;
        }
    }
    if (w)
    {
        if (s || !currentHideAllControls)
        {
            currentPlaylistState = m_pMainWindow->ui->playlistLayoutWidget->isVisible();
        }
        m_pMainWindow->ui->menuBarWidget->hide();

        m_pMainWindow->ui->playbackLayoutWidget->hide();
        m_pMainWindow->ui->seekBar->hide();
        m_pMainWindow->ui->centralWidget->layout()->removeWidget(m_pMainWindow->ui->playbackLayoutWidget);
        m_pMainWindow->ui->playbackLayoutWidget->setParent(m_pMainWindow);
        m_pMainWindow->ui->playbackLayoutWidget->move(QPoint(0, m_pMainWindow->geometry().height() -
                                                             m_pMainWindow->ui->playbackLayoutWidget->height()));
        m_pMainWindow->ui->centralWidget->layout()->removeWidget(m_pMainWindow->ui->seekBar);
        m_pMainWindow->ui->seekBar->setParent(m_pMainWindow);
        m_pMainWindow->ui->seekBar->move(QPoint(0, m_pMainWindow->geometry().height() -
                                                m_pMainWindow->ui->playbackLayoutWidget->height() -
                                                m_pMainWindow->ui->seekBar->height()));

        if (fullscreenProgressIndicator)
        {
            fullscreenProgressIndicator->close();
            delete fullscreenProgressIndicator;
            fullscreenProgressIndicator = nullptr;
        }
        if (currentShowFullscreenIndicator)
        {
            fullscreenProgressIndicator = new ProgressIndicatorBar(m_pMainWindow);
            fullscreenProgressIndicator->setFixedSize(QSize(m_pMainWindow->width(),
                                                            (2.0 / 1080.0) * m_pMainWindow->height()));
            fullscreenProgressIndicator->move(QPoint(0, m_pMainWindow->height() -
                                                     fullscreenProgressIndicator->height()));
            fullscreenProgressIndicator->setRange(0, 1000);
            fullscreenProgressIndicator->show();
        }

        m_pMainWindow->mouseMoveEvent(new QMouseEvent(QMouseEvent::MouseMove, QCursor::pos(),
                                       Qt::NoButton, Qt::NoButton, Qt::NoModifier));
    }
    else
    {
        if (fullscreenProgressIndicator)
        {
            fullscreenProgressIndicator->close();
            delete fullscreenProgressIndicator;
            fullscreenProgressIndicator = nullptr;
        }

        m_pMainWindow->ui->seekBar->setParent(m_pMainWindow->ui->centralWidget);
        m_pMainWindow->ui->centralWidget->layout()->addWidget(m_pMainWindow->ui->seekBar);
        m_pMainWindow->ui->playbackLayoutWidget->setParent(m_pMainWindow->ui->centralWidget);
        m_pMainWindow->ui->centralWidget->layout()->addWidget(m_pMainWindow->ui->playbackLayoutWidget);

        if (currentMenuVisible)
        {
            m_pMainWindow->ui->menuBarWidget->show();
        }
        m_pMainWindow->ui->seekBar->show();
        m_pMainWindow->ui->playbackLayoutWidget->show();
        //setCursor(QCursor(Qt::ArrowCursor)); // show cursor
        //autohide->stop();
        showPlaylist(currentPlaylistState);
    }
}

void PlaybackManager::mainWindowShowFullScreen(bool fs)
{
    static Qt::WindowStates oldState;
    if (fs)
    {
        if (sugoi->dimDialog && sugoi->dimDialog->isVisible())
        {
            sugoi->Dim(false);
        }
        if (m_pMainWindow->ui->menuBarWidget && m_pMainWindow->ui->menuBarWidget->isVisible())
        {
            m_pMainWindow->ui->menuBarWidget->hide();
        }
        if (m_pMainWindow->ui->titleBarWidget->isVisible())
        {
            m_pMainWindow->ui->titleBarWidget->hide();
        }
        oldState = m_pMainWindow->windowState();
        m_pMainWindow->showFullScreen();
        if (!currentHideAllControls)
        {
            hideAllControls(true, false);
        }
    }
    else
    {
        m_pMainWindow->setWindowState(oldState);
        if (!m_pMainWindow->ui->titleBarWidget->isVisible())
        {
            m_pMainWindow->ui->titleBarWidget->show();
        }
        if (m_pMainWindow->ui->menuBarWidget && !m_pMainWindow->ui->menuBarWidget->isVisible())
        {
            m_pMainWindow->ui->menuBarWidget->show();
        }
        if(!currentHideAllControls)
        {
            hideAllControls(false, false);
        }
    }
}

void PlaybackManager::showPlaylist(bool visible)
{
    if (m_pMainWindow->ui->splitter->position() != 0 && visible) // ignore showing if it's already visible as it resets original position
    {
        return;
    }

    if (visible)
    {
        m_pMainWindow->ui->splitter->setPosition(m_pMainWindow->ui->splitter->normalPosition()); // bring splitter position to normal
    }
    else
    {
        if (m_pMainWindow->ui->splitter->position() != m_pMainWindow->ui->splitter->max() && m_pMainWindow->ui->splitter->position() != 0)
        {
            m_pMainWindow->ui->splitter->setNormalPosition(m_pMainWindow->ui->splitter->position()); // save current splitter position as the normal position
        }
        m_pMainWindow->ui->splitter->setPosition(0); // set splitter position to right-most
        m_pMainWindow->setFocus();
    }
}

void PlaybackManager::hideAlbumArt(bool hide)
{
    if (hide)
    {
        if (m_pMainWindow->ui->splitter->position() != m_pMainWindow->ui->splitter->max() && m_pMainWindow->ui->splitter->position() != 0)
        {
            m_pMainWindow->ui->splitter->setNormalPosition(m_pMainWindow->ui->splitter->position()); // save splitter position as the normal position
        }
        m_pMainWindow->ui->splitter->setPosition(m_pMainWindow->ui->splitter->max()); // bring the splitter position to the left-most
    }
    else
    {
        m_pMainWindow->ui->splitter->setPosition(m_pMainWindow->ui->splitter->normalPosition()); // bring the splitter to normal position
    }
}

void PlaybackManager::updateRecentFiles()
{
    m_pMainWindow->ui->menu_Recently_Opened->clear();
    QAction *action = nullptr;
    int n = 1;
    int N = recent.length();
    for (auto &f : recent)
    {
        action = m_pMainWindow->ui->menu_Recently_Opened->addAction(QString("%0. %1").arg(Util::FormatNumberWithAmpersand(n, N), Util::ShortenPathToParent(f).replace("&","&&")));
        if (n++ == 1)
        {
            action->setShortcut(QKeySequence("Ctrl+Z"));
        }
        connect(action, &QAction::triggered,
                [=]
                {
                    m_pMpvObject->load(f);
                });
    }
}

void PlaybackManager::setPlayButtonIcon(bool play)
{
    if (play)
    {
        m_pMainWindow->ui->playButton->setIcon(QIcon(":/images/default_play.svg"));
        m_pMainWindow->ui->action_Play->setText(tr("&Play"));
#ifdef Q_OS_WIN
        m_pMainWindow->playpause_toolbutton->setToolTip(tr("Play"));
        m_pMainWindow->playpause_toolbutton->setIcon(QIcon(":/images/tool-play.ico"));
        m_pMainWindow->taskbarButton->setOverlayIcon(QIcon(":/images/tool-pause.ico"));
        m_pMainWindow->taskbarProgress->show();
        m_pMainWindow->taskbarProgress->pause();
#endif
    }
    else
    {
        m_pMainWindow->ui->playButton->setIcon(QIcon(":/images/default_pause.svg"));
        m_pMainWindow->ui->action_Play->setText(tr("&Pause"));
#ifdef Q_OS_WIN
        m_pMainWindow->playpause_toolbutton->setToolTip(tr("Pause"));
        m_pMainWindow->playpause_toolbutton->setIcon(QIcon(":/images/tool-pause.ico"));
        m_pMainWindow->taskbarButton->setOverlayIcon(QIcon(":/images/tool-play.ico"));
        m_pMainWindow->taskbarProgress->show();
        m_pMainWindow->taskbarProgress->resume();
#endif
    }
}

void PlaybackManager::setNextButtonEnabled(bool enable)
{
    m_pMainWindow->ui->nextButton->setEnabled(enable);
    m_pMainWindow->ui->actionPlay_Next_File->setEnabled(enable);
#ifdef Q_OS_WIN
    m_pMainWindow->next_toolbutton->setEnabled(enable);
#endif
}

void PlaybackManager::setPreviousButtonEnabled(bool enable)
{
    m_pMainWindow->ui->previousButton->setEnabled(enable);
    m_pMainWindow->ui->actionPlay_Previous_File->setEnabled(enable);
#ifdef Q_OS_WIN
    m_pMainWindow->prev_toolbutton->setEnabled(enable);
#endif
}

void PlaybackManager::setRemainingLabels(int time)
{
    // todo: move setVisible functions outside of this function which gets called every second and somewhere at the start of a video
    const Mpv::FileInfo &fi = m_pMpvObject->fileInfo();
    if (fi.duration == 0)
    {
        if (m_pMainWindow->ui->remainingLabel->isVisible())
        {
            m_pMainWindow->ui->remainingLabel->hide();
        }
        if (m_pMainWindow->ui->seperatorLabel->isVisible())
        {
            m_pMainWindow->ui->seperatorLabel->hide();
        }

        m_pMainWindow->ui->durationLabel->setText(Util::FormatTime(time, time));
    }
    else
    {
        if (m_pMainWindow->ui->remainingLabel->isHidden())
        {
            m_pMainWindow->ui->remainingLabel->show();
        }
        if (m_pMainWindow->ui->seperatorLabel->isHidden())
        {
            m_pMainWindow->ui->seperatorLabel->show();
        }

        m_pMainWindow->ui->durationLabel->setText(Util::FormatTime(time, fi.length));
        if (currentRemaining)
        {
            int remainingTime = fi.duration - time;
            QString text = "-" + Util::FormatTime(remainingTime, fi.length);
            if (m_pMpvObject->speed() != 1.0)
            {
                double speed = m_pMpvObject->speed();
                text += QString("  (-%0)").arg(Util::FormatTime(int(remainingTime/speed), int(fi.duration/speed)));
            }
            m_pMainWindow->ui->remainingLabel->setText(text);
        }
        else
        {
            QString text = Util::FormatTime(fi.duration, fi.duration);
            if (m_pMpvObject->speed() != 1.0)
            {
                double speed = m_pMpvObject->speed();
                text += QString("  (%0)").arg(Util::FormatTime(int(fi.length/speed), int(fi.length/speed)));
            }
            m_pMainWindow->ui->remainingLabel->setText(text);
        }
    }
}

bool PlaybackManager::isPlayingMusic() const
{
    if (m_pMpvObject->playState() > 0)
    {
        QFileInfo fi(m_pMpvObject->file());
        QString suffix = QString::fromLatin1("*.") + fi.suffix();
        if (Mpv::audio_filetypes.contains(suffix))
        {
            return true;
        }
    }
    return false;
}

bool PlaybackManager::isPlayingVideo() const
{
    if (m_pMpvObject->playState() > 0)
    {
        QFileInfo fi(m_pMpvObject->file());
        QString suffix = QString::fromLatin1("*.") + fi.suffix();
        if (Mpv::video_filetypes.contains(suffix))
        {
            return true;
        }
    }
    return false;
}

void PlaybackManager::setIndexLabels(bool enable)
{
    int i = m_pMainWindow->ui->playlistWidget->currentRow();
    int index = m_pMainWindow->ui->playlistWidget->CurrentIndex();

    // next file
    if (enable && index + 1 < m_pMainWindow->ui->playlistWidget->count()) // not the last entry
    {
        setNextButtonEnabled(true);
        m_pMainWindow->ui->nextButton->setIndex(index+2); // starting at 1 instead of at 0 like actual index
    }
    else
    {
        setNextButtonEnabled(false);
    }

    // previous file
    if (enable && index - 1 >= 0) // not the first entry
    {
        setPreviousButtonEnabled(true);
        m_pMainWindow->ui->previousButton->setIndex(-index); // we use a negative index value for the left button
    }
    else
    {
        setPreviousButtonEnabled(false);
    }

    if (i == -1) // no selection
    {
        m_pMainWindow->ui->indexLabel->setText(tr("No selection"));
        m_pMainWindow->ui->indexLabel->setEnabled(false);
    }
    else
    {
        m_pMainWindow->ui->indexLabel->setEnabled(true);
        m_pMainWindow->ui->indexLabel->setText(tr("%0 / %1").arg(QString::number(i+1), QString::number(m_pMainWindow->ui->playlistWidget->count())));
    }
}

void PlaybackManager::setPlaybackControls(bool enable)
{
    // playback controls
    m_pMainWindow->ui->seekBar->setEnabled(enable);
    m_pMainWindow->ui->rewindButton->setEnabled(enable);

    setIndexLabels(enable);

    // menubar
    m_pMainWindow->ui->action_Stop->setEnabled(enable);
    m_pMainWindow->ui->action_Restart->setEnabled(enable);
    m_pMainWindow->ui->menuS_peed->setEnabled(enable);
    m_pMainWindow->ui->action_Jump_to_Time->setEnabled(enable);
    m_pMainWindow->ui->actionMedia_Info->setEnabled(enable);
    m_pMainWindow->ui->actionShow_in_Folder->setEnabled(enable && !m_pMpvObject->path().isEmpty());
    m_pMainWindow->ui->action_Full_Screen->setEnabled(enable);
    if (!enable)
    {
        m_pMainWindow->ui->action_Hide_Album_Art->setEnabled(false);
        m_pMainWindow->ui->menuSubtitle_Track->setEnabled(false);
        m_pMainWindow->ui->menuFont_Si_ze->setEnabled(false);
    }
}

void PlaybackManager::togglePlaylist()
{
    showPlaylist(!isPlaylistVisible());
}

bool PlaybackManager::isPlaylistVisible() const
{
    // if the position is 0, playlist is hidden
    return (m_pMainWindow->ui->splitter->position() != 0);
}

void PlaybackManager::load(const QString &path)
{
    if (path.isEmpty())
    {
        return;
    }
    if (m_pMpvObject == nullptr)
    {
        return;
    }
    if (m_pMainWindow == nullptr)
    {
        return;
    }
    if (!m_pMainWindow->isActiveWindow())
    {
        activateMainWindow();
    }
    m_pMpvObject->load(path);
}

MpvObject *PlaybackManager::mpvObject() const
{
    return m_pMpvObject;
}

MainWindow *PlaybackManager::mainWindow() const
{
    return m_pMainWindow;
}

bool PlaybackManager::fullScreenMode() const
{
    if (m_pMainWindow == nullptr)
    {
        return false;
    }
    return (currentHideAllControls || m_pMainWindow->isFullScreen());
}

void PlaybackManager::connectMpvSignalsAndSlots()
{
    if (m_pMpvObject == nullptr || m_pMainWindow == nullptr)
    {
        return;
    }

    connect(m_pMpvObject, &MpvObject::hwdecChanged,
            [=](bool enable)
            {
                if (enable)
                {
                    m_pMainWindow->ui->hwdecButton->setIcon(QIcon(":/images/default_hwdec.svg"));
                }
                else
                {
                    m_pMainWindow->ui->hwdecButton->setIcon(QIcon(":/images/disabled_hwdec.svg"));
                }
            });

    //connect(m_pMpvObject, &MpvObject::playlistChanged, this, &MainWindow::playlistChanged);

    connect(m_pMpvObject, &MpvObject::fileInfoChanged,
            [=](const Mpv::FileInfo &fileInfo)
            {
                if (m_pMpvObject->playState() > 0)
                {
                    if (fileInfo.media_title.isEmpty())
                    {
                        m_pMainWindow->setWindowTitle2("Sugoi Player");
                    }
                    else if (fileInfo.media_title == "-")
                    {
                        m_pMainWindow->setWindowTitle2("Sugoi Player: stdin"); // todo: disable playlist?
                    }
                    else
                    {
                        m_pMainWindow->setWindowTitle2(fileInfo.media_title);
                    }

                    QString f = m_pMpvObject->file(), file = m_pMpvObject->path() + f;
                    if(!f.isEmpty() && currentMaxRecent > 0)
                    {
                        int i = recent.indexOf(file);
                        if (i >= 0)
                        {
                            int t = recent.at(i).time;
                            if (t > 0 && currentResume)
                            {
                                m_pMpvObject->seek(t);
                            }
                            recent.removeAt(i);
                        }
                        if (recent.isEmpty() || recent.front() != file)
                        {
                            updateRecentFiles(); // update after initialization and only if the current file is different from the first recent
                            while (recent.length() > currentMaxRecent - 1)
                            {
                                recent.removeLast();
                            }
                            recent.push_front(
                                Recent(file,
                                       (m_pMpvObject->path().isEmpty() || !Util::IsValidFile(file)) ?
                                           fileInfo.media_title : QString()));
                            currentRecent = &recent.front();
                        }
                    }

                    // reset speed if length isn't known and we have a streaming video
                    // todo: don't save this reset, put their speed back when a normal video comes on
                    // todo: disable speed alteration during streaming media
                    if (fileInfo.duration == 0)
                        if (m_pMpvObject->speed() != 1.0)
                            m_pMpvObject->setSpeed(1.0);

                    m_pMainWindow->ui->seekBar->setTracking(fileInfo.duration);

                    if (m_pMainWindow->ui->actionMedia_Info->isChecked())
                    {
                        sugoi->MediaInfo(true);
                    }

                    setRemainingLabels(fileInfo.duration);

                    if (sugoi->sysTrayIcon->isVisible() && !currentHidePopup)
                    {
                        QString title = QString();
                        if (m_pMpvObject->fileInfo().metadata.contains("artist"))
                        {
                            title = m_pMpvObject->fileInfo().metadata["artist"];
                        }
                        if (title.isEmpty())
                        {
                            if (m_pMpvObject->fileInfo().metadata.contains("author"))
                            {
                                title = m_pMpvObject->fileInfo().metadata["author"];
                            }
                        }
                        sugoi->sysTrayIcon->showMessage(title, m_pMpvObject->fileInfo().media_title, QIcon(":/images/player.svg"), 4000);
                    }
                }
            });

    connect(m_pMpvObject, &MpvObject::trackListChanged,
            [=](const QList<Mpv::Track> &trackList)
            {
                if (m_pMpvObject->playState() > 0)
                {
                    QAction *action = nullptr;
                    bool video = false;
                    bool albumArt = false;

                    m_pMainWindow->ui->menuSubtitle_Track->clear();
                    m_pMainWindow->ui->menuSubtitle_Track->addAction(m_pMainWindow->ui->action_Add_Subtitle_File);
                    m_pMainWindow->ui->menuAudio_Tracks->clear();
                    m_pMainWindow->ui->menuAudio_Tracks->addAction(m_pMainWindow->ui->action_Add_Audio_File);
                    for (auto &track : trackList)
                    {
                        if (track.type == "sub")
                        {
                            action = m_pMainWindow->ui->menuSubtitle_Track->addAction(QString("%0: %1 (%2)").arg(QString::number(track.id), track.title, track.lang + (track.external ? "*" : "")).replace("&", "&&"));
                            connect(action, &QAction::triggered,
                                    [=]
                                    {
                                        // basically, if you uncheck the selected subtitle id, we hide subtitles
                                        // when you check a subtitle id, we make sure subtitles are showing and set it
                                        if (m_pMpvObject->sid() == track.id)
                                        {
                                            if (m_pMpvObject->subtitleVisibility())
                                            {
                                                m_pMpvObject->showSubtitles(false);
                                                return;
                                            }
                                            else
                                            {
                                                m_pMpvObject->showSubtitles(true);
                                            }
                                        }
                                        else if (!m_pMpvObject->subtitleVisibility())
                                        {
                                            m_pMpvObject->showSubtitles(true);
                                        }
                                        m_pMpvObject->setSid(track.id);
                                        showText(QString("%0 %1: %2 (%3)").arg(tr("Sub"), QString::number(track.id), track.title, track.lang + (track.external ? "*" : "")));
                                    });
                        }
                        else if (track.type == "audio")
                        {
                            action = m_pMainWindow->ui->menuAudio_Tracks->addAction(QString("%0: %1 (%2)").arg(QString::number(track.id), track.title, track.lang).replace("&", "&&"));
                            connect(action, &QAction::triggered,
                                    [=]
                                    {
                                        if (m_pMpvObject->aid() != track.id) // don't allow selection of the same track
                                        {
                                            m_pMpvObject->setAid(track.id);
                                            showText(QString("%0 %1: %2 (%3)").arg(tr("Audio"), QString::number(track.id), track.title, track.lang));
                                        }
                                        else
                                        {
                                            action->setChecked(true); // recheck the track
                                        }
                                    });
                        }
                        else if (track.type == "video") // video track
                        {
                            if (!track.albumart) // isn't album art
                            {
                                video = true;
                            }
                            else
                            {
                                albumArt = true;
                            }
                        }
                    }
                    if (video)
                    {
                        // if we were hiding album art, show it--we've gone to a video
                        if (!m_pMainWindow->ui->mpvFrame->styleSheet().isEmpty()) // remove filler album art
                        {
                            m_pMainWindow->ui->mpvFrame->setStyleSheet("");
                        }
                        if (m_pMainWindow->ui->action_Hide_Album_Art->isChecked())
                        {
                            hideAlbumArt(false);
                        }
                        m_pMainWindow->ui->action_Hide_Album_Art->setEnabled(false);
                        m_pMainWindow->ui->menuSubtitle_Track->setEnabled(true);
                        if (m_pMainWindow->ui->menuSubtitle_Track->actions().count() > 1)
                        {
                            m_pMainWindow->ui->menuFont_Si_ze->setEnabled(true);
                            m_pMainWindow->ui->actionShow_Subtitles->setEnabled(true);
                            m_pMainWindow->ui->actionShow_Subtitles->setChecked(m_pMpvObject->subtitleVisibility());
                        }
                        else
                        {
                            m_pMainWindow->ui->menuFont_Si_ze->setEnabled(false);
                            m_pMainWindow->ui->actionShow_Subtitles->setEnabled(false);
                            m_pMainWindow->ui->actionShow_Subtitles->setChecked(false);
                        }
                        m_pMainWindow->ui->menuTake_Screenshot->setEnabled(true);
                        m_pMainWindow->ui->menuFit_Window->setEnabled(true);
                        m_pMainWindow->ui->menuAspect_Ratio->setEnabled(true);
                        m_pMainWindow->ui->action_Frame_Step->setEnabled(true);
                        m_pMainWindow->ui->actionFrame_Back_Step->setEnabled(true);
                        m_pMainWindow->ui->action_Deinterlace->setEnabled(true);
                        m_pMainWindow->ui->action_Motion_Interpolation->setEnabled(true);
                    }
                    else
                    {
                        if (!albumArt)
                        {
                            // put in filler albumArt
                            if (m_pMainWindow->ui->mpvFrame->styleSheet().isEmpty())
                            {
                                m_pMainWindow->ui->mpvFrame->setStyleSheet("background-image:url(:/images/album-art.png);background-repeat:no-repeat;background-position:center;");
                            }
                        }
                        m_pMainWindow->ui->action_Hide_Album_Art->setEnabled(true);
                        m_pMainWindow->ui->menuSubtitle_Track->setEnabled(false);
                        m_pMainWindow->ui->menuFont_Si_ze->setEnabled(false);
                        m_pMainWindow->ui->actionShow_Subtitles->setEnabled(false);
                        m_pMainWindow->ui->actionShow_Subtitles->setChecked(false);
                        m_pMainWindow->ui->menuTake_Screenshot->setEnabled(false);
                        m_pMainWindow->ui->menuFit_Window->setEnabled(false);
                        m_pMainWindow->ui->menuAspect_Ratio->setEnabled(false);
                        m_pMainWindow->ui->action_Frame_Step->setEnabled(false);
                        m_pMainWindow->ui->actionFrame_Back_Step->setEnabled(false);
                        m_pMainWindow->ui->action_Deinterlace->setEnabled(false);
                        m_pMainWindow->ui->action_Motion_Interpolation->setEnabled(false);
                    }

                    if (m_pMainWindow->ui->menuAudio_Tracks->actions().count() == 1)
                    {
                        m_pMainWindow->ui->menuAudio_Tracks->actions().first()->setEnabled(false);
                    }

                    if (pathChanged && autoFit)
                    {
                        sugoi->FitWindow(autoFit, false);
                        pathChanged = false;
                    }
                }
            });

    connect(m_pMpvObject, &MpvObject::chaptersChanged,
            [=](const QList<Mpv::Chapter> &chapters)
            {
                if (m_pMpvObject->playState() > 0)
                {
                    QAction *action = nullptr;
                    QList<int> ticks;
                    int n = 1;
                    int N = chapters.length();
                    m_pMainWindow->ui->menu_Chapters->clear();
                    for (auto &ch : chapters)
                    {
                        action = m_pMainWindow->ui->menu_Chapters->addAction(QString("%0: %1").arg(Util::FormatNumberWithAmpersand(n, N), ch.title));
                        if (n <= 9)
                        {
                            action->setShortcut(QKeySequence("Ctrl+"+QString::number(n)));
                        }
                        connect(action, &QAction::triggered,
                                [=]
                                {
                                    m_pMpvObject->seek(ch.time);
                                });
                        ticks.push_back(ch.time);
                        n++;
                    }
                    if (m_pMainWindow->ui->menu_Chapters->actions().count() == 0)
                    {
                        m_pMainWindow->ui->menu_Chapters->setEnabled(false);
                        m_pMainWindow->ui->action_Next_Chapter->setEnabled(false);
                        m_pMainWindow->ui->action_Previous_Chapter->setEnabled(false);
                    }
                    else
                    {
                        m_pMainWindow->ui->menu_Chapters->setEnabled(true);
                        m_pMainWindow->ui->action_Next_Chapter->setEnabled(true);
                        m_pMainWindow->ui->action_Previous_Chapter->setEnabled(true);
                    }

                    m_pMainWindow->ui->seekBar->setTicks(ticks);
                }
            });

    connect(m_pMpvObject, &MpvObject::playStateChanged,
            [=](Mpv::PlayState playState)
            {
                switch (playState)
                {
                case Mpv::Loaded:
                    showText(tr("Loading..."), 0);
                    break;
                case Mpv::Started:
                    if (!init) // will only happen the first time a file is loaded.
                    {
                        m_pMainWindow->ui->hwdecButton->setEnabled(true);
                        m_pMainWindow->ui->action_Play->setEnabled(true);
                        m_pMainWindow->ui->playButton->setEnabled(true);
#ifdef Q_OS_WIN
                        playpause_toolbutton->setEnabled(true);
#endif
                        m_pMainWindow->ui->playlistButton->setEnabled(true);
                        m_pMainWindow->ui->action_Show_Playlist->setEnabled(true);
                        m_pMainWindow->ui->menuAudio_Tracks->setEnabled(true);
                        init = true;
                    }
                    setPlaybackControls(true);
                    m_pMpvObject->play();
                    sugoi->overlay->showStatusText(QString(), 0);
                case Mpv::Playing:
                    setPlayButtonIcon(false);
                    if (onTop == "playing")
                    {
                        Util::SetAlwaysOnTop(m_pMainWindow, true);
                    }
                    break;
                case Mpv::Paused:
                case Mpv::Stopped:
                    setPlayButtonIcon(true);
                    if (onTop == "playing")
                    {
                        Util::SetAlwaysOnTop(m_pMainWindow, false);
                    }
                    break;
                case Mpv::Idle:
                    if (init)
                    {
                        if (m_pMainWindow->ui->action_This_File->isChecked()) // repeat this file
                        {
                            if (m_pMainWindow->isVisible() || playInBackground)
                            {
                                m_pMainWindow->ui->playlistWidget->PlayIndex(0, true); // restart file
                            }
                        }
                        else if (m_pMainWindow->ui->actionStop_after_Current->isChecked() ||  // stop after playing this file
                                m_pMainWindow->ui->playlistWidget->CurrentIndex() >= m_pMainWindow->ui->playlistWidget->count()-1) // end of the playlist
                        {
                            if (!m_pMainWindow->ui->actionStop_after_Current->isChecked() && // not supposed to stop after current
                                m_pMainWindow->ui->action_Playlist->isChecked() && // we're supposed to restart the playlist
                                m_pMainWindow->ui->playlistWidget->count() > 0) // playlist isn't empty
                            {
                                if (m_pMainWindow->isVisible() || playInBackground)
                                {
                                    m_pMainWindow->ui->playlistWidget->PlayIndex(0); // restart playlist
                                }
                            }
                            else // stop
                            {
                                m_pMainWindow->setWindowTitle2("Sugoi Player");
                                setPlaybackControls(false);
                                m_pMainWindow->ui->seekBar->setTracking(0);
                                m_pMainWindow->ui->actionStop_after_Current->setChecked(false);
                                if (!m_pMainWindow->ui->mpvFrame->styleSheet().isEmpty()) // remove filler album art
                                {
                                    m_pMainWindow->ui->mpvFrame->setStyleSheet("");
                                }
                            }
                        }
                        else
                        {
                            if (m_pMainWindow->isVisible() || playInBackground)
                            {
                                m_pMainWindow->ui->playlistWidget->PlayIndex(1, true);
                            }
                        }
                    }
                    break;
                }
            });

    connect(m_pMpvObject, &MpvObject::pathChanged,
            [=]
            {
                pathChanged = true;
            });

    connect(m_pMpvObject, &MpvObject::fileChanging,
              [=](int t, int l)
              {
                  if (currentRecent != nullptr)
                  {
                      if (t > 0.05 * l && t < 0.95 * l) // only save if within the middle 90%
                      {
                          current->time = t;
                      }
                      else
                      {
                          current->time = 0;
                      }
                  }
              });

    connect(m_pMpvObject, &MpvObject::timeChanged,
            [=](int i)
            {
                const Mpv::FileInfo &fi = m_pMpvObject->fileInfo();
                // set the seekBar's location with NoSignal function so that it doesn't trigger a seek
                // the formula is a simple ratio seekBar's max * time/totalTime
                double currentPercent = (double)i/fi.length;
                currentPercent = currentPercent >= 0.0 ? currentPercent : 0.0;
                m_pMainWindow->ui->seekBar->setValueNoSignal(m_pMainWindow->ui->seekBar->maximum()*currentPercent);

                taskbarProgress->setValue(taskbarProgress->maximum()*currentPercent);

                if (fullscreenProgressIndicator)
                {
                    fullscreenProgressIndicator->setValue(fullscreenProgressIndicator->maximum()*currentPercent);
                }

                setRemainingLabels(i);

                // set next/previous chapter's enabled state
                if (fi.chapters.length() > 0)
                {
                    m_pMainWindow->ui->action_Next_Chapter->setEnabled(i < fi.chapters.last().time);
                    m_pMainWindow->ui->action_Previous_Chapter->setEnabled(i > fi.chapters.first().time);
                }
            });

    connect(m_pMpvObject, &MpvObject::volumeChanged, m_pMainWindow->ui->volumeSlider, &CustomSlider::setValueNoSignal);

    connect(m_pMpvObject, &MpvObject::speedChanged,
            [=](double speed)
            {
                static double last = 1;
                if (last != speed)
                {
                    if (init)
                    {
                        showText(tr("Speed: %0x").arg(QString::number(speed)));
                    }
                    if (speed <= 0.25)
                    {
                        m_pMainWindow->ui->action_Decrease->setEnabled(false);
                    }
                    else
                    {
                        m_pMainWindow->ui->action_Decrease->setEnabled(true);
                    }
                    last = speed;
                }
            });

    connect(m_pMpvObject, &MpvObject::sidChanged,
            [=](int sid)
            {
                QList<QAction*> actions = m_pMainWindow->ui->menuSubtitle_Track->actions();
                for (auto &action : actions)
                {
                    if (action->text().startsWith(QString::number(sid)))
                    {
                        action->setCheckable(true);
                        action->setChecked(true);
                    }
                    else
                    {
                        action->setChecked(false);
                    }
                }
            });

    connect(m_pMpvObject, &MpvObject::aidChanged,
            [=](int aid)
            {
                QList<QAction*> actions = m_pMainWindow->ui->menuAudio_Tracks->actions();
                for (auto &action : actions)
                {
                    if (action->text().startsWith(QString::number(aid)))
                    {
                        action->setCheckable(true);
                        action->setChecked(true);
                    }
                    else
                    {
                        action->setChecked(false);
                    }
                }
            });

    connect(m_pMpvObject, &MpvObject::subtitleVisibilityChanged,
            [=](bool b)
            {
                if (m_pMainWindow->ui->actionShow_Subtitles->isEnabled())
                {
                    m_pMainWindow->ui->actionShow_Subtitles->setChecked(b);
                }
                if (init)
                {
                    showText(b ? tr("Subtitles visible") : tr("Subtitles hidden"));
                }
            });

    connect(m_pMpvObject, &MpvObject::muteChanged,
            [=](bool b)
            {
                if (b)
                {
                    m_pMainWindow->ui->muteButton->setIcon(QIcon(":/images/default_mute.svg"));
                }
                else
                {
                    m_pMainWindow->ui->muteButton->setIcon(QIcon(":/images/default_unmute.svg"));
                }
                showText(b ? tr("Muted") : tr("Unmuted"));
            });

    connect(m_pMpvObject, &MpvObject::voChanged,
            [=](QString vo)
            {
                m_pMainWindow->ui->action_Motion_Interpolation->setChecked(vo.contains("interpolation"));
            });
}

void PlaybackManager::connectMainWindowUiSignalsAndSlots()
{
    connect(m_pMainWindow->ui->minimizeButton, &QPushButton::clicked,
            [=]
            {
                m_pMainWindow->showMinimized();
            });

    connect(m_pMainWindow->ui->maximizeButton, &QPushButton::clicked,
            [=]
            {
                if (m_pMainWindow->isMaximized())
                {
                    m_pMainWindow->showNormal();
                    m_pMainWindow->ui->maximizeButton->setIcon(QIcon(":/images/disabled_maximize.svg"));
                }
                else
                {
                    m_pMainWindow->showMaximized();
                    m_pMainWindow->ui->maximizeButton->setIcon(QIcon(":/images/disabled_restore.svg"));
                }
            });

    connect(m_pMainWindow->ui->closeButton, &QPushButton::clicked,
            [=]
            {
                m_pMainWindow->close();
            });

    connect(m_pMainWindow->ui->hwdecButton, &QPushButton::clicked,
            [=]
            {
                m_pMpvObject->setHwdec(!m_pMpvObject->hwdec(), true);
            });

    connect(m_pMainWindow->ui->playlistButton, &QPushButton::clicked, this, &MainWindow::togglePlaylist);

    connect(m_pMainWindow->ui->seekBar, &SeekBar::valueChanged,                        // Playback: Seekbar clicked
            [=](int i)
            {
                m_pMpvObject->seek(m_pMpvObject->relative(((double)i/m_pMainWindow->ui->seekBar->maximum())*m_pMpvObject->fileInfo().duration), true);
            });

    connect(m_pMainWindow->ui->openButton, &OpenButton::LeftClick, sugoi, &SugoiEngine::Open);
    connect(m_pMainWindow->ui->openButton, &OpenButton::MiddleClick, sugoi, &SugoiEngine::Jump);
    connect(m_pMainWindow->ui->openButton, &OpenButton::RightClick, sugoi, &SugoiEngine::OpenLocation);

    connect(m_pMainWindow->ui->rewindButton, &QPushButton::clicked,                    // Playback: Rewind button
            [=]
            {
                m_pMpvObject->rewind();
            });

    connect(m_pMainWindow->ui->previousButton, &IndexButton::clicked,                  // Playback: Previous button
            [=]
            {
                m_pMainWindow->ui->playlistWidget->PlayIndex(-1, true);
            });

    connect(m_pMainWindow->ui->playButton, &QPushButton::clicked, sugoi, &SugoiEngine::PlayPause);

    connect(m_pMainWindow->ui->nextButton, &IndexButton::clicked,                      // Playback: Next button
            [=]
            {
                m_pMainWindow->ui->playlistWidget->PlayIndex(1, true);
            });

    connect(m_pMainWindow->ui->muteButton, &QPushButton::clicked,
            [=]
            {
                m_pMpvObject->setMute(!m_pMpvObject->mute());
            });

    connect(m_pMainWindow->ui->volumeSlider, &CustomSlider::valueChanged,              // Playback: Volume slider adjusted
            [=](int i)
            {
                m_pMpvObject->setVolume(i, true);
            });

    connect(m_pMainWindow->ui->splitter, &CustomSplitter::positionChanged,             // Splitter position changed
            [=](int i)
            {
                blockSignals(true);
                if (i == 0) // right-most, playlist is hidden
                {
                    m_pMainWindow->ui->action_Show_Playlist->setChecked(false);
                    m_pMainWindow->ui->action_Hide_Album_Art->setChecked(false);
                    m_pMainWindow->ui->playlistLayoutWidget->setVisible(false);
                }
                else if (i == ui->splitter->max()) // left-most, album art is hidden, playlist is visible
                {
                    m_pMainWindow->ui->action_Show_Playlist->setChecked(true);
                    m_pMainWindow->ui->action_Hide_Album_Art->setChecked(true);
                }
                else // in the middle, album art is visible, playlist is visible
                {
                    m_pMainWindow->ui->action_Show_Playlist->setChecked(true);
                    m_pMainWindow->ui->action_Hide_Album_Art->setChecked(false);
                }
                m_pMainWindow->ui->playlistLayoutWidget->setVisible(m_pMainWindow->ui->action_Show_Playlist->isChecked());
                blockSignals(false);
                if (m_pMainWindow->ui->actionMedia_Info->isChecked())
                {
                    sugoi->overlay->showInfoText();
                }
            });

    connect(m_pMainWindow->ui->searchBox, &QLineEdit::textChanged,                     // Playlist: Search box
            [=](QString s)
            {
                m_pMainWindow->ui->playlistWidget->Search(s);
            });

    connect(m_pMainWindow->ui->indexLabel, &CustomLabel::clicked,                      // Playlist: Clicked the indexLabel
            [=]
            {
                QString res = InputDialog::getInput(tr("Enter the file number you want to play:\nNote: Value must be from %0 - %1").arg("1", QString::number(ui->playlistWidget->count())),
                                                    tr("Enter File Number"),
                                                    [m_pMainWindow](QString input)
                                                    {
                                                        int in = input.toInt();
                                                        if (in >= 1 && in <= ui->playlistWidget->count())
                                                        {
                                                            return true;
                                                         }
                                                        return false;
                                                    },
                                                    m_pMainWindow);
                if (!res.isEmpty())
                {
                    m_pMainWindow->ui->playlistWidget->PlayIndex(res.toInt()-1); // user index will be 1 greater than actual
                }
            });

    connect(m_pMainWindow->ui->playlistWidget, &PlaylistWidget::currentRowChanged,     // Playlist: Playlist selection changed
            [=](int)
            {
                setIndexLabels(true);
            });

    connect(m_pMainWindow->ui->currentFileButton, &QPushButton::clicked,               // Playlist: Select current file button
            [=]
            {
                m_pMainWindow->ui->playlistWidget->SelectIndex(m_pMainWindow->ui->playlistWidget->CurrentIndex());
            });

    connect(m_pMainWindow->ui->refreshButton, &QPushButton::clicked,                   // Playlist: Refresh playlist button
            [=]
            {
                m_pMainWindow->ui->playlistWidget->RefreshPlaylist();
            });

    connect(m_pMainWindow->ui->inputLineEdit, &CustomLineEdit::submitted,
            [=](QString s)
            {
                sugoi->Command(s);
                m_pMainWindow->ui->inputLineEdit->setText("");
    });
}

void PlaybackManager::connectMainWindowOtherSignalsAndSlots()
{
    commandActionMap = {
        {"mpv add chapter +1", m_pMainWindow->ui->action_Next_Chapter},
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
                    // load Qt translations
                    if(sugoi->qtTranslator != nullptr)
                    {
                        qApp->removeTranslator(sugoi->qtTranslator);
                        delete sugoi->qtTranslator;
                        sugoi->qtTranslator = nullptr;
                    }
                    sugoi->qtTranslator = new QTranslator();
#ifdef _STATIC_BUILD
                    QString langPath = QApplication::applicationDirPath() + QDir::separator() + QString::fromLatin1("translations");
#else
                    QString langPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
#endif
                    if (sugoi->qtTranslator->load(QString("qt_%0").arg(l), langPath))
                    {
                        qApp->installTranslator(sugoi->qtTranslator);
                    }
                    else
                    {
                        delete sugoi->qtTranslator;
                        sugoi->qtTranslator = nullptr;
                    }

                    // load application translations
                    if(sugoi->translator != nullptr)
                    {
                        qApp->removeTranslator(sugoi->translator);
                        delete sugoi->translator;
                        sugoi->translator = nullptr;
                    }
                    sugoi->translator = new QTranslator(qApp);
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
                    if(sugoi->qtTranslator != nullptr)
                    {
                        qApp->removeTranslator(sugoi->qtTranslator);
                        delete sugoi->qtTranslator;
                        sugoi->qtTranslator = nullptr;
                    }
                }

                // save strings we want to keep
                QString title = windowTitle(),
                        duration = ui->durationLabel->text(),
                        remaining = ui->remainingLabel->text(),
                        index = ui->indexLabel->text();

                ui->retranslateUi(this);

                // reload strings we kept
                SetWindowTitle2(title);
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
                if (isHidden())
                {
                    return;
                }
                if (sugoi->sysTrayIcon != nullptr)
                {
                    sugoi->sysTrayIcon->setVisible(visible);
                }
            });

    connect(this, &MainWindow::quickStartModeChanged,
            [=](bool quickStart)
            {
                if (isHidden())
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
                    QtConcurrent::run([=]
                    {
                        mpv->LoadFile(filePath);
                    });
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

void PlaybackManager::mapShortcuts()
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
