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
    SetParent(reinterpret_cast<HWND>(m_pMainWindow->winId()), GetDesktopWindow());
    m_pMpvObject = new MpvObject(m_pMainWindow);
    m_pPropertiesWindow = new PropertiesWindow(m_pMainWindow);

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
        m_pMainWindow->setSysTrayIconVisibility(false);
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
        m_pMainWindow->BringWindowToFront();
    }
    m_pMainWindow->openFileFromCmd(path);
}

MpvObject *PlaybackManager::mpvObject() const
{
    return m_pMpvObject;
}

MainWindow *PlaybackManager::mainWindow() const
{
    return m_pMainWindow;
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

    connect(m_pMpvObject, &MpvObject::playlistChanged, this, &MainWindow::playlistChanged);

    connect(m_pMpvObject, &MpvObject::fileInfoChanged,
            [=](const Mpv::FileInfo &fileInfo)
            {
                if(m_pMpvObject->getPlayState() > 0)
                {
                    if(fileInfo.media_title == "")
                        m_pMainWindow->setWindowTitle2("Sugoi Player");
                    else if(fileInfo.media_title == "-")
                        m_pMainWindow->setWindowTitle2("Sugoi Player: stdin"); // todo: disable playlist?
                    else
                        m_pMainWindow->setWindowTitle2(fileInfo.media_title);

                    QString f = m_pMpvObject->getFile(), file = m_pMpvObject->getPath()+f;
                    if(f != QString() && maxRecent > 0)
                    {
                        int i = recent.indexOf(file);
                        if(i >= 0)
                        {
                            int t = recent.at(i).time;
                            if(t > 0 && resume)
                                m_pMpvObject->Seek(t);
                            recent.removeAt(i);
                        }
                        if(recent.isEmpty() || recent.front() != file)
                        {
                            UpdateRecentFiles(); // update after initialization and only if the current file is different from the first recent
                            while(recent.length() > maxRecent-1)
                                recent.removeLast();
                            recent.push_front(
                                Recent(file,
                                       (m_pMpvObject->getPath() == QString() || !Util::IsValidFile(file)) ?
                                           fileInfo.media_title : QString()));
                            current = &recent.front();
                        }
                    }

                    // reset speed if length isn't known and we have a streaming video
                    // todo: don't save this reset, put their speed back when a normal video comes on
                    // todo: disable speed alteration during streaming media
                    if(fileInfo.length == 0)
                        if(m_pMpvObject->getSpeed() != 1)
                            m_pMpvObject->Speed(1);

                    m_pMainWindow->ui->seekBar->setTracking(fileInfo.length);

                    if(m_pMainWindow->ui->actionMedia_Info->isChecked())
                        sugoi->MediaInfo(true);

                    SetRemainingLabels(fileInfo.length);

                    if(sugoi->sysTrayIcon->isVisible() && !hidePopup)
                    {
                        QString title = QString();
                        if (m_pMpvObject->getFileInfo().metadata.contains("artist"))
                        {
                            title = m_pMpvObject->getFileInfo().metadata["artist"];
                        }
                        if (title.isEmpty())
                        {
                            if (m_pMpvObject->getFileInfo().metadata.contains("author"))
                            {
                                title = m_pMpvObject->getFileInfo().metadata["author"];
                            }
                        }
                        sugoi->sysTrayIcon->showMessage(title, m_pMpvObject->getFileInfo().media_title, QIcon(":/images/player.svg"), 4000);
                    }
                }
            });

    connect(m_pMpvObject, &MpvObject::trackListChanged,
            [=](const QList<Mpv::Track> &trackList)
            {
                if(m_pMpvObject->getPlayState() > 0)
                {
                    QAction *action;
                    bool video = false,
                         albumArt = false;

                    m_pMainWindow->ui->menuSubtitle_Track->clear();
                    m_pMainWindow->ui->menuSubtitle_Track->addAction(m_pMainWindow->ui->action_Add_Subtitle_File);
                    m_pMainWindow->ui->menuAudio_Tracks->clear();
                    m_pMainWindow->ui->menuAudio_Tracks->addAction(m_pMainWindow->ui->action_Add_Audio_File);
                    for(auto &track : trackList)
                    {
                        if(track.type == "sub")
                        {
                            action = m_pMainWindow->ui->menuSubtitle_Track->addAction(QString("%0: %1 (%2)").arg(QString::number(track.id), track.title, track.lang + (track.external ? "*" : "")).replace("&", "&&"));
                            connect(action, &QAction::triggered,
                                    [=]
                                    {
                                        // basically, if you uncheck the selected subtitle id, we hide subtitles
                                        // when you check a subtitle id, we make sure subtitles are showing and set it
                                        if(m_pMpvObject->getSid() == track.id)
                                        {
                                            if(m_pMpvObject->getSubtitleVisibility())
                                            {
                                                m_pMpvObject->ShowSubtitles(false);
                                                return;
                                            }
                                            else
                                                m_pMpvObject->ShowSubtitles(true);
                                        }
                                        else if(!m_pMpvObject->getSubtitleVisibility())
                                            m_pMpvObject->ShowSubtitles(true);
                                        m_pMpvObject->Sid(track.id);
                                        m_pMpvObject->ShowText(QString("%0 %1: %2 (%3)").arg(tr("Sub"), QString::number(track.id), track.title, track.lang + (track.external ? "*" : "")));
                                    });
                        }
                        else if(track.type == "audio")
                        {
                            action = m_pMainWindow->ui->menuAudio_Tracks->addAction(QString("%0: %1 (%2)").arg(QString::number(track.id), track.title, track.lang).replace("&", "&&"));
                            connect(action, &QAction::triggered,
                                    [=]
                                    {
                                        if(m_pMpvObject->getAid() != track.id) // don't allow selection of the same track
                                        {
                                            m_pMpvObject->Aid(track.id);
                                            m_pMpvObject->ShowText(QString("%0 %1: %2 (%3)").arg(tr("Audio"), QString::number(track.id), track.title, track.lang));
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
                        if(m_pMainWindow->ui->mpvFrame->styleSheet() != QString()) // remove filler album art
                            m_pMainWindow->ui->mpvFrame->setStyleSheet("");
                        if(m_pMainWindow->ui->action_Hide_Album_Art->isChecked())
                            HideAlbumArt(false);
                        m_pMainWindow->ui->action_Hide_Album_Art->setEnabled(false);
                        m_pMainWindow->ui->menuSubtitle_Track->setEnabled(true);
                        if(m_pMainWindow->ui->menuSubtitle_Track->actions().count() > 1)
                        {
                            m_pMainWindow->ui->menuFont_Si_ze->setEnabled(true);
                            m_pMainWindow->ui->actionShow_Subtitles->setEnabled(true);
                            m_pMainWindow->ui->actionShow_Subtitles->setChecked(m_pMpvObject->getSubtitleVisibility());
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
                        if(!albumArt)
                        {
                            // put in filler albumArt
                            if(m_pMainWindow->ui->mpvFrame->styleSheet() == QString())
                                m_pMainWindow->ui->mpvFrame->setStyleSheet("background-image:url(:/images/album-art.png);background-repeat:no-repeat;background-position:center;");
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

                    if(m_pMainWindow->ui->menuAudio_Tracks->actions().count() == 1)
                        m_pMainWindow->ui->menuAudio_Tracks->actions().first()->setEnabled(false);

                    if(pathChanged && autoFit)
                    {
                        sugoi->FitWindow(autoFit, false);
                        pathChanged = false;
                    }
                }
            });

    connect(m_pMpvObject, &MpvObject::chaptersChanged,
            [=](const QList<Mpv::Chapter> &chapters)
            {
                if(m_pMpvObject->getPlayState() > 0)
                {
                    QAction *action;
                    QList<int> ticks;
                    int n = 1,
                        N = chapters.length();
                    m_pMainWindow->ui->menu_Chapters->clear();
                    for(auto &ch : chapters)
                    {
                        action = m_pMainWindow->ui->menu_Chapters->addAction(QString("%0: %1").arg(Util::FormatNumberWithAmpersand(n, N), ch.title));
                        if(n <= 9)
                            action->setShortcut(QKeySequence("Ctrl+"+QString::number(n)));
                        connect(action, &QAction::triggered,
                                [=]
                                {
                                    m_pMpvObject->Seek(ch.time);
                                });
                        ticks.push_back(ch.time);
                        n++;
                    }
                    if(m_pMainWindow->ui->menu_Chapters->actions().count() == 0)
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
                switch(playState)
                {
                case Mpv::Loaded:
                    sugoi->m_pMpvObject->ShowText(tr("Loading..."), 0);
                    break;

                case Mpv::Started:
                    if(!init) // will only happen the first time a file is loaded.
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
                    SetPlaybackControls(true);
                    m_pMpvObject->Play();
                    sugoi->overlay->showStatusText(QString(), 0);
                case Mpv::Playing:
                    SetPlayButtonIcon(false);
                    if(onTop == "playing")
                        Util::SetAlwaysOnTop(this, true);
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
                        if(m_pMainWindow->ui->action_This_File->isChecked()) // repeat this file
                        {
                            if (isVisible() || playInBackground)
                            {
                                m_pMainWindow->ui->playlistWidget->PlayIndex(0, true); // restart file
                            }
                        }
                        else if(m_pMainWindow->ui->actionStop_after_Current->isChecked() ||  // stop after playing this file
                                m_pMainWindow->ui->playlistWidget->CurrentIndex() >= m_pMainWindow->ui->playlistWidget->count()-1) // end of the playlist
                        {
                            if(!m_pMainWindow->ui->actionStop_after_Current->isChecked() && // not supposed to stop after current
                                m_pMainWindow->ui->action_Playlist->isChecked() && // we're supposed to restart the playlist
                                m_pMainWindow->ui->playlistWidget->count() > 0) // playlist isn't empty
                            {
                                if (isVisible() || playInBackground)
                                {
                                    m_pMainWindow->ui->playlistWidget->PlayIndex(0); // restart playlist
                                }
                            }
                            else // stop
                            {
                                m_pMainWindow->setWindowTitle2("Sugoi Player");
                                SetPlaybackControls(false);
                                m_pMainWindow->ui->seekBar->setTracking(0);
                                m_pMainWindow->ui->actionStop_after_Current->setChecked(false);
                                if(m_pMainWindow->ui->mpvFrame->styleSheet() != QString()) // remove filler album art
                                    m_pMainWindow->ui->mpvFrame->setStyleSheet("");
                            }
                        }
                        else
                        {
                            if (isVisible() || playInBackground)
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
                  if(current != nullptr)
                  {
                      if(t > 0.05*l && t < 0.95*l) // only save if within the middle 90%
                          current->time = t;
                      else
                          current->time = 0;
                  }
              });

    connect(m_pMpvObject, &MpvObject::timeChanged,
            [=](int i)
            {
                const Mpv::FileInfo &fi = m_pMpvObject->getFileInfo();
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

                SetRemainingLabels(i);

                // set next/previous chapter's enabled state
                if(fi.chapters.length() > 0)
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
                if(last != speed)
                {
                    if(init)
                        m_pMpvObject->ShowText(tr("Speed: %0x").arg(QString::number(speed)));
                    if(speed <= 0.25)
                        m_pMainWindow->ui->action_Decrease->setEnabled(false);
                    else
                        m_pMainWindow->ui->action_Decrease->setEnabled(true);
                    last = speed;
                }
            });

    connect(m_pMpvObject, &MpvObject::sidChanged,
            [=](int sid)
            {
                QList<QAction*> actions = m_pMainWindow->ui->menuSubtitle_Track->actions();
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

    connect(m_pMpvObject, &MpvObject::aidChanged,
            [=](int aid)
            {
                QList<QAction*> actions = m_pMainWindow->ui->menuAudio_Tracks->actions();
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

    connect(m_pMpvObject, &MpvObject::subtitleVisibilityChanged,
            [=](bool b)
            {
                if(m_pMainWindow->ui->actionShow_Subtitles->isEnabled())
                    m_pMainWindow->ui->actionShow_Subtitles->setChecked(b);
                if(init)
                    m_pMpvObject->ShowText(b ? tr("Subtitles visible") : tr("Subtitles hidden"));
            });

    connect(m_pMpvObject, &MpvObject::muteChanged,
            [=](bool b)
            {
                if(b)
                    m_pMainWindow->ui->muteButton->setIcon(QIcon(":/images/default_mute.svg"));
                else
                    m_pMainWindow->ui->muteButton->setIcon(QIcon(":/images/default_unmute.svg"));
                m_pMpvObject->ShowText(b ? tr("Muted") : tr("Unmuted"));
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
                m_pMpvObject->Hwdec(!m_pMpvObject->getHwdec(), true);
            });

    connect(m_pMainWindow->ui->playlistButton, &QPushButton::clicked, this, &MainWindow::TogglePlaylist);

    connect(m_pMainWindow->ui->seekBar, &SeekBar::valueChanged,                        // Playback: Seekbar clicked
            [=](int i)
            {
                mpv->Seek(mpv->Relative(((double)i/ui->seekBar->maximum())*mpv->getFileInfo().length), true);
            });

    connect(m_pMainWindow->ui->openButton, &OpenButton::LeftClick, sugoi, &SugoiEngine::Open);
    connect(m_pMainWindow->ui->openButton, &OpenButton::MiddleClick, sugoi, &SugoiEngine::Jump);
    connect(m_pMainWindow->ui->openButton, &OpenButton::RightClick, sugoi, &SugoiEngine::OpenLocation);

    connect(m_pMainWindow->ui->rewindButton, &QPushButton::clicked,                    // Playback: Rewind button
            [=]
            {
                m_pMpvObject->Rewind();
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
                mpv->Mute(!mpv->getMute());
            });

    connect(m_pMainWindow->ui->volumeSlider, &CustomSlider::valueChanged,              // Playback: Volume slider adjusted
            [=](int i)
            {
                mpv->Volume(i, true);
            });

    connect(m_pMainWindow->ui->splitter, &CustomSplitter::positionChanged,             // Splitter position changed
            [=](int i)
            {
                blockSignals(true);
                if(i == 0) // right-most, playlist is hidden
                {
                    m_pMainWindow->ui->action_Show_Playlist->setChecked(false);
                    m_pMainWindow->ui->action_Hide_Album_Art->setChecked(false);
                    m_pMainWindow->ui->playlistLayoutWidget->setVisible(false);
                }
                else if(i == ui->splitter->max()) // left-most, album art is hidden, playlist is visible
                {
                    m_pMainWindow->ui->action_Show_Playlist->setChecked(true);
                    m_pMainWindow->ui->action_Hide_Album_Art->setChecked(true);
                }
                else // in the middle, album art is visible, playlist is visible
                {
                    m_pMainWindow->ui->action_Show_Playlist->setChecked(true);
                    m_pMainWindow->ui->action_Hide_Album_Art->setChecked(false);
                }
                m_pMainWindow->ui->playlistLayoutWidget->setVisible(ui->action_Show_Playlist->isChecked());
                blockSignals(false);
                if(m_pMainWindow->ui->actionMedia_Info->isChecked())
                    sugoi->overlay->showInfoText();
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
                                                    [this](QString input)
                                                    {
                                                        int in = input.toInt();
                                                        if(in >= 1 && in <= ui->playlistWidget->count())
                                                            return true;
                                                        return false;
                                                    },
                                                    this);
                if(res != "")
                    m_pMainWindow->ui->playlistWidget->PlayIndex(res.toInt()-1); // user index will be 1 greater than actual
            });

    connect(m_pMainWindow->ui->playlistWidget, &PlaylistWidget::currentRowChanged,     // Playlist: Playlist selection changed
            [=](int)
            {
                SetIndexLabels(true);
            });

    connect(m_pMainWindow->ui->currentFileButton, &QPushButton::clicked,               // Playlist: Select current file button
            [=]
            {
                m_pMainWindow->ui->playlistWidget->SelectIndex(ui->playlistWidget->CurrentIndex());
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
