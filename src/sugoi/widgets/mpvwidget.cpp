#include "mpvwidget.h"

#include <stdexcept>

#include <QOpenGLContext>
#include <QMetaObject>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileInfoList>
#include <QDateTime>
#include <QVBoxLayout>
#include <QThread>

#include "sugoiengine.h"
#include "overlayhandler.h"
#include "util.h"
#include "widgets/logowidget.h"

MpvObject::MpvObject(QObject *parent, const QString &clientName) : QObject(parent)
{
    currentWorker = new QThread(this);
    currentWorker->start();

    currentController = new MpvController(this);
    currentController->moveToThread(worker);

    connect(this, &MpvObject::mpvCommand,
            [=](const QVariant &params)
            {
                if (currentController == nullptr)
                {
                    return;
                }
                QMetaObject::invokeMethod(currentController, "mpvCommand", Qt::QueuedConnection,
                                          Q_ARG(QVariant, params));
            });
    connect(this, &MpvObject::mpvSetOption,
            [=](const QString &name, const QVariant &value)
            {
                if (currentController == nullptr)
                {
                    return;
                }
                QMetaObject::invokeMethod(currentController, "mpvSetOption", Qt::QueuedConnection,
                                          Q_ARG(QString, name),
                                          Q_ARG(QVariant, value));
            });
    connect(this, &MpvObject::mpvSetProperty,
            [=](const QString &name, const QVariant &value)
            {
                if (currentController == nullptr)
                {
                    return;
                }
                QMetaObject::invokeMethod(currentController, "mpvSetProperty", Qt::QueuedConnection,
                                          Q_ARG(QString, name),
                                          Q_ARG(QVariant, value));
            });
    connect(this, &MpvObject::sugoiCommand,
            [=](const QString &cmd, const QVariant &params)
            {
                if (currentController == nullptr)
                {
                    return;
                }
                QMetaObject::invokeMethod(currentController, cmd.toUtf8().constData(), Qt::QueuedConnection,
                                          Q_ARG(QVariant, params));
            });

    QMetaObject::invokeMethod(currentController, "create", Qt::BlockingQueuedConnection);

    connect(currentWorker, &QThread::finished, currentController, &MpvController::deleteLater);
}

MpvObject::~MpvObject()
{
    if (currentWidget)
    {
        delete currentWidget;
        currentWidget = nullptr;
    }
    currentWorker->deleteLater();
}

void MpvObject::setHostWindow(QObject *newHostWindow)
{
    if (newHostWindow == nullptr)
    {
        return;
    }
    QWidget *newHostWidget = qobject_cast<QWidget *>(newHostWindow);
    if (newHostWidget == currentHostWindow)
    {
        return;
    }
    currentHostWindow = newHostWidget;
}

void MpvObject::setRendererType(Mpv::Renderers newRendererType)
{
    if (currentHostWindow == nullptr)
    {
        return;
    }
    if (currentRendererType == newRendererType)
    {
        return;
    }
    currentRendererType = newRendererType;
    if (currentWidget != nullptr)
    {
        delete currentWidget;
        currentWidget = nullptr;
    }
    switch (currentRendererType)
    {
    case Mpv::Renderers::Null:
        currentWidget = nullptr;
        break;
    case Mpv::Renderers::GL:
        currentWidget = new MpvGLWidget(currentHostWindow);
        break;
    case Mpv::Renderers::Vulkan:
        currentWidget = new MpvVulkanWidget(currentHostWindow);
        break;
    case Mpv::Renderers::D3D:
        currentWidget = new MpvD3DWidget(currentHostWindow);
        break;
    case Mpv::Renderers::D2D:
        currentWidget = new MpvD2DWidget(currentHostWindow);
        break;
    case Mpv::Renderers::GDI:
        currentWidget = new MpvGDIWidget(currentHostWindow);
        break;
    default:
        currentWidget = nullptr;
        break;
    }
    if (currentWidget == nullptr)
    {
        return;
    }
    if (currentHostWindow->layout() == nullptr)
    {
        QVBoxLayout *newVBLayout = new QVBoxLayout(currentHostWindow);
        currentHostWindow->setLayout(newVBLayout);
    }
    currentHostWindow->layout()->setContentsMargins(0, 0, 0, 0);
    currentHostWindow->layout()->setSpacing(0);
    mpvWidget()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    currentHostWindow->layout()->addWidget(mpvWidget());
    currentHostWindow->setController(currentController);
    currentHostWindow->initMpv();
}

QString MpvObject::mpvVersion() const
{
    return mpvProperty(QLatin1String("mpv-version")).toString();
}

MpvController *MpvObject::mpvController() const
{
    return currentController;
}

QWidget *MpvObject::mpvWidget() const
{
    return currentWidget->self();
}

QVariant MpvObject::mpvProperty(const QString &name) const
{
    if (currentController == nullptr)
    {
        return QVariant();
    }
    QVariant propertyVal;
    QMetaObject::invokeMethod(currentController, "mpvProperty", Qt::QueuedConnection,
                              Q_RETURN_ARG(QVariant, propertyVal),
                              Q_ARG(QString, name));
    return propertyVal;
}

QVariant MpvObject::sugoiProperty(const QString &name) const
{
    if (currentController == nullptr)
    {
        return QVariant();
    }
    QVariant propertyVal;
    QMetaObject::invokeMethod(currentController, name.toUtf8().constData(), Qt::QueuedConnection,
                              Q_RETURN_ARG(QVariant, propertyVal));
    return propertyVal;
}

void MpvObject::setLogoUrl(const QString &filename)
{
    currentWidget->setLogoUrl(filename);
}

void MpvObject::setLogoBackground(const QColor &color)
{
    currentWidget->setLogoBackground(color);
}


MpvWidgetInterface::MpvWidgetInterface(MpvObject *object) : mpvObject(object)
{
}

MpvWidgetInterface::~MpvWidgetInterface()
{
    currentController = nullptr;
}

void MpvWidgetInterface::setController(MpvController *newController)
{
    currentController = newController;
}

void MpvWidgetInterface::setLogoUrl(const QString &filename)
{
    Q_UNUSED(filename);
}

void MpvWidgetInterface::setLogoBackground(const QColor &color)
{
    Q_UNUSED(color);
}

void MpvWidgetInterface::setDrawLogo(bool yes)
{
    Q_UNUSED(yes);
}


static void *mpvGetProcAddress(void *ctx, const char *name)
{
    Q_UNUSED(ctx);
    QOpenGLContext *glctx = QOpenGLContext::currentContext();
    if (!glctx)
    {
        return nullptr;
    }
    return (void *)glctx->getProcAddress(QByteArray(name));
}

MpvGLWidget::MpvGLWidget(MpvObject *object, QWidget *parent) :
    QOpenGLWidget(parent), MpvWidgetInterface(object)
{
    connect(this, &QOpenGLWidget::frameSwapped, this, &MpvGLWidget::self_frameSwapped);
    connect(currentController, &MpvController::playbackStarted, this, &MpvGLWidget::self_playbackStarted);
    connect(currentController, &MpvController::playbackFinished, this, &MpvGLWidget::self_playbackFinished);
    connect(this, &MpvGLWidget::mouseMoved, currentMpvObject, &MpvObject::mouseMoved);
    connect(this, &MpvGLWidget::mousePressed, currentMpvObject, &MpvObject::mousePressed);
}

MpvGLWidget::~MpvGLWidget()
{
    makeCurrent();
    if (glMpv)
    {
        mpv_opengl_cb_set_update_callback(glMpv, nullptr, nullptr);
        mpv_opengl_cb_uninit_gl(glMpv);
    }
    if (logo)
    {
        delete logo;
        logo = nullptr;
    }
}

QWidget *MpvGLWidget::self()
{
    return this;
}

void MpvGLWidget::initMpv()
{
    // grab a copy of the mpvGl draw context
    QMetaObject::invokeMethod(currentController, "mpvDrawContext", Qt::BlockingQueuedConnection,
                              Q_RETURN_ARG(mpv_opengl_cb_context *, glMpv));

    // ask mpv to make draw requests to us
    mpv_opengl_cb_set_update_callback(glMpv, MpvGLWidget::onMpvUpdate, (void *)this);
}

void MpvGLWidget::setLogoUrl(const QString &filename)
{
    makeCurrent();
    if (!logo)
    {
        logo = new LogoDrawer(this);
        connect(logo, &LogoDrawer::logoSize, currentMpvObject, &MpvObject::logoSizeChanged);
    }
    logo->setLogoUrl(filename);
    logo->resizeGL(width(), height());
    if (drawLogo)
    {
        update();
    }
    doneCurrent();
}

void MpvGLWidget::setLogoBackground(const QColor &color)
{
    logo->setLogoBackground(color);
}

void MpvGLWidget::setDrawLogo(bool yes)
{
    drawLogo = yes;
    update();
}

void MpvGLWidget::initializeGL()
{
    if (mpv_opengl_cb_init_gl(glMpv, nullptr, mpvGetProcAddress, nullptr) < 0)
    {
        throw std::runtime_error("could not initialize OpenGL.");
    }

    if (!logo)
    {
        logo = new LogoDrawer(this);
    }
}

void MpvGLWidget::paintGL()
{
    if (!drawLogo)
    {
        mpv_opengl_cb_draw(glMpv, defaultFramebufferObject(), glWidth, -glHeight);
    }
    else
    {
        logo->paintGL(this);
    }
}

void MpvGLWidget::resizeGL(int w, int h)
{
    qreal r = devicePixelRatio();
    glWidth = int(w * r);
    glHeight = int(h * r);
    logo->resizeGL(width(),height());
}

void MpvGLWidget::mouseMoveEvent(QMouseEvent *event)
{
    Q_EMIT mouseMoved(event->x(), event->y());
    QOpenGLWidget::mouseMoveEvent(event);
}

void MpvGLWidget::mousePressEvent(QMouseEvent *event)
{
    Q_EMIT mousePressed(event->x(), event->y());
    QOpenGLWidget::mousePressEvent(event);
}

void MpvGLWidget::onMpvUpdate(void *ctx)
{
    QMetaObject::invokeMethod(reinterpret_cast<MpvGLWidget *>(ctx), "maybeUpdate");
}

void MpvGLWidget::maybeUpdate()
{
    if (window()->isMinimized())
    {
        makeCurrent();
        paintGL();
        context()->swapBuffers(context()->surface());
        self_frameSwapped();
        doneCurrent();
    }
    else
    {
        update();
    }
}

void MpvGLWidget::self_frameSwapped()
{
    if (!drawLogo)
    {
        mpv_opengl_cb_report_flip(glMpv, 0);
    }
}

void MpvGLWidget::self_playbackStarted()
{
    drawLogo = false;
}

void MpvGLWidget::self_playbackFinished()
{
    drawLogo = true;
    update();
}


MpvController::MpvController(QObject *parent) : QObject(parent),
    glMpv(nullptr)
{
}

MpvController::~MpvController()
{
    mpv_set_wakeup_callback(mpv, nullptr, nullptr);
}

void MpvController::mpvCommand(const QVariant &params)
{
    mpv::qt::command_variant(mpv, params);
}

void MpvController::mpvSetOption(const QString &name, const QVariant &value)
{
    mpv::qt::set_option_variant(mpv, name, value);
}

void MpvController::mpvSetProperty(const QString &name, const QVariant &value)
{
    mpv::qt::set_property_variant(mpv, name, value);
}

QVariant MpvController::mpvProperty(const QString &name) const
{
    return mpv::qt::get_property_variant(mpv, name);
}

void MpvController::create()
{
    mpv = mpv::qt::Handle::FromRawHandle(mpv_create());
    if (!mpv)
    {
        throw std::runtime_error("could not create mpv context.");
    }

    mpvSetOption(QLatin1String("input-default-bindings"), QLatin1String("no")); // disable mpv default key bindings
    mpvSetOption(QLatin1String("input-vo-keyboard"), QLatin1String("no")); // disable keyboard input on the X11 window
    mpvSetOption(QLatin1String("input-cursor"), QLatin1String("no")); // no mouse handling
    mpvSetOption(QLatin1String("cursor-autohide"), QLatin1String("no")); // no cursor-autohide, we handle that
    mpvSetOption(QLatin1String("ytdl"), QLatin1String("yes")); // youtube-dl support

    if (mpv_initialize(mpv) < 0)
    {
        throw std::runtime_error("could not initialize mpv context");
    }

    // Make use of the MPV_SUB_API_OPENGL_CB API.
    mpvSetOption(QLatin1String("vo"), QLatin1String("opengl-cb"));

    glMpv = (mpv_opengl_cb_context *)mpv_get_sub_api(mpv, MPV_SUB_API_OPENGL_CB);
    if (!glMpv)
    {
        throw std::runtime_error("OpenGL not compiled in");
    }

    mpv_observe_property(mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "percent-pos", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "playback-time", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "time-remaining", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "playtime-remaining", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "ao-volume", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "sid", MPV_FORMAT_INT64);
    mpv_observe_property(mpv, 0, "aid", MPV_FORMAT_INT64);
    mpv_observe_property(mpv, 0, "sub-visibility", MPV_FORMAT_FLAG);
    mpv_observe_property(mpv, 0, "ao-mute", MPV_FORMAT_FLAG);
    mpv_observe_property(mpv, 0, "core-idle", MPV_FORMAT_FLAG);
    mpv_observe_property(mpv, 0, "paused-for-cache", MPV_FORMAT_FLAG);

    mpv_set_wakeup_callback(mpv, MpvController::mpvWakeup, this);
}

QString MpvController::clientName() const
{
    return QString::fromUtf8(mpv_client_name(mpv));
}

unsigned long MpvController::apiVersion() const
{
    return mpv_client_api_version();
}

mpv_opengl_cb_context* MpvController::mpvDrawContext() const
{
    return glMpv;
}

Mpv::FileInfo MpvController::fileInfo() const
{
    return currentFileInfo;
}

Mpv::PlayState MpvController::playState() const
{
    return currentPlayState;
}

QString MpvController::file() const
{
    return currentFile;
}

QString MpvController::path() const
{
    return currentPath;
}

QString MpvController::fullPath() const
{
    return currentFullPath;
}

QString MpvController::screenshotFormat() const
{
    return currentScreenshotFormat;
}

QString MpvController::screenshotTemplate() const
{
    return currentScreenshotTemplate;
}

QString MpvController::screenshotDirectory() const
{
    return currentScreenshotDirectory;
}

QString MpvController::vo() const
{
    return currentVo;
}

QString MpvController::msgLevel() const
{
    return currentMsgLevel;
}

double MpvController::speed() const
{
    return currentSpeed;
}

double MpvController::time() const
{
    return currentTime;
}

double MpvController::volume() const
{
    return currentVolume;
}

int MpvController::vid() const
{
    return currentVid;
}

int MpvController::aid() const
{
    return currentAid;
}

int MpvController::sid() const
{
    return currentSid;
}

bool MpvController::subtitleVisibility() const
{
    return currentSubtitleVisibility;
}

bool MpvController::mute() const
{
    return currentMute;
}

double MpvController::position() const
{
    return currentPosition;
}

bool MpvController::hwdec() const
{
    return currentHwdec;
}

double MpvController::percent() const
{
    return currentPercent;
}

int MpvController::osdWidth() const
{
    return currentOsdWidth;
}

int MpvController::osdHeight() const
{
    return currentOsdHeight;
}

bool MpvController::deinterlace() const
{
    return currentDeinterlace;
}

QString MpvController::aspect() const
{
    return currentAspect;
}

QString MpvController::mediaInfo() const
{
    QFileInfo fi(currentFullPath);

    double avsync = mpvProperty(QLatin1String("avsync")).toDouble();
    double fps = mpvProperty(QLatin1String("estimated-vf-fps")).toDouble();
    double vbitrate = mpvProperty(QLatin1String("video-bitrate")).toDouble();
    double abitrate = mpvProperty(QLatin1String("audio-bitrate")).toDouble();
    QString current_vo = mpvProperty(QLatin1String("current-vo")).toString();
    QString current_ao = mpvProperty(QLatin1String("current-ao")).toString();
    QString hwdec_active = mpvProperty(QLatin1String("hwdec-current")).toString();

    int vtracks = 0, atracks = 0;

    for (auto &track : currentFileInfo.tracks)
    {
        if (track.type == QLatin1String("video"))
        {
            ++vtracks;
        }
        else if (track.type == QLatin1String("audio"))
        {
            ++atracks;
        }
    }

    const QString outer = "%0: %1\r\n", inner = "    %0: %1\r\n";

    QString out = outer.arg(tr("File"), fi.fileName()) +
            inner.arg(tr("Title"), currentFileInfo.media_title) +
            inner.arg(tr("File size"), Util::HumanSize(fi.size())) +
            inner.arg(tr("Date created"), fi.created().toString()) +
            inner.arg(tr("Media length"), Util::FormatTime(currentFileInfo.duration, currentFileInfo.duration)) + "\r\n";

    if (!currentFileInfo.video_params.codec.isEmpty())
    {
        out += outer.arg(tr("Video (x%0)").arg(QString::number(vtracks)), currentFileInfo.video_params.codec) +
                inner.arg(tr("Video Output"), QString("%0 (hwdec %1)").arg(current_vo, hwdec_active)) +
                inner.arg(tr("Resolution"), QString("%0 x %1 (%2)").arg(QString::number(currentFileInfo.video_params.width),
                                                                        QString::number(currentFileInfo.video_params.height),
                                                                        Util::Ratio(currentFileInfo.video_params.width, currentFileInfo.video_params.height))) +
                inner.arg(tr("FPS"), QString::number(fps)) +
                inner.arg(tr("A/V Sync"), QString::number(avsync)) +
                inner.arg(tr("Bitrate"), tr("%0 kbps").arg(vbitrate/1000)) + "\r\n";
    }
    if (!currentFileInfo.audio_params.codec.isEmpty())
    {
        out += outer.arg(tr("Audio (x%0)").arg(QString::number(atracks)), currentFileInfo.audio_params.codec) +
                inner.arg(tr("Audio Output"), current_ao) +
                inner.arg(tr("Sample Rate"), QString::number(currentFileInfo.audio_params.samplerate)) +
                inner.arg(tr("Channels"), QString::number(currentFileInfo.audio_params.channels)) +
                inner.arg(tr("Bitrate"), tr("%0 kbps").arg(abitrate)) + "\r\n";
    }

    if (currentFileInfo.chapters.length() > 0)
    {
        out += outer.arg(tr("Chapters"), QString());
        int n = 1;
        for (auto &chapter : currentFileInfo.chapters)
        {
            out += inner.arg(QString::number(n++), chapter.title);
        }
        out += "\r\n";
    }

    if (currentFileInfo.metadata.size() > 0)
    {
        out += outer.arg(tr("Metadata"), QString());
        for (auto data = currentFileInfo.metadata.begin(); data != currentFileInfo.metadata.end(); ++data)
        {
            out += inner.arg(data.key(), *data);
        }
        out += "\r\n";
    }

    return out;
}

void MpvController::parseMpvEvents()
{
    // Process all events, until the event queue is empty.
    while (mpv)
    {
        mpv_event *event = mpv_wait_event(mpv, 0);
        if (event->event_id == MPV_EVENT_NONE)
        {
            break;
        }
        HandleErrorCode(event->error);
        handleMpvEvent(event);
    }
}

void MpvController::load(const QString &file)
{
    play(loadPlaylist(file));
}

QString MpvController::loadPlaylist(const QString &file) const
{
    if (file.isEmpty()) // ignore empty file name
    {
        return QString();
    }

    if (file == "-")
    {
        currentPath = QString();
        emit pathChanged(currentPath);
        currentFullPath = file;
        emit fullPathChanged(currentFullPath);
        emit playlistChanged(QStringList() << f);
    }
    else if (Util::IsValidUrl(file)) // web url
    {
        currentPath = QString();
        emit pathChanged(currentPath);
        currentFullPath = file;
        emit fullPathChanged(currentFullPath);
        emit playlistChanged(QStringList() << f);
    }
    else // local file
    {
        QFileInfo fi(file);
        if (!fi.exists()) // file doesn't exist
        {
            emit showText(tr("File does not exist")); // tell the user
            return QString(); // don't do anything more
        }
        else if (fi.isDir()) // if directory
        {
            // set new path
            currentPath = fi.absoluteFilePath();
            emit pathChanged(currentPath);
            currentFullPath = currentPath;
            emit fullPathChanged(currentFullPath);
            return populatePlaylist();
        }
        else if (fi.isFile()) // if file
        {
            // set new path
            currentPath = fi.absolutePath();
            emit pathChanged(currentPath);
            currentFullPath = fi.absoluteFilePath();
            emit fullPathChanged(currentFullPath);
            populatePlaylist();
            return fi.fileName();
        }
    }
    return file;
}

bool MpvController::play(const QString &file) const
{
    if (file.isEmpty()) // ignore if file doesn't exist
    {
        return false;
    }

    if (currentPath.isEmpty()) // web url
    {
        open(file);
        currentFile = file;
        emit fileChanged(currentFile);
    }
    else
    {
        QFile qf(currentPath + QDir::separator() + file);
        if (qf.exists())
        {
            currentFullPath = currentPath + QDir::separator() + file;
            emit fullPathChanged(currentFullPath);
            open(currentFullPath);
            currentFile = file;
            emit fileChanged(currentFile);
            play();
        }
        else
        {
            emit showText(tr("File no longer exists")); // tell the user
            return false;
        }
    }
    return true;
}

void MpvController::play()
{
    if (currentPlayState > 0 && mpv)
    {
        mpvSetProperty(QLatin1String("pause"), false);
    }
}

void MpvController::pause()
{
    if (currentPlayState > 0 && mpv)
    {
        mpvSetProperty(QLatin1String("pause"), true);
    }
}

void MpvController::stop()
{
    restart();
    pause();
    //mpvCommand(QLatin1String("stop"));
}

void MpvController::playPause(const QString &fileIfStopped)
{
    if (currentPlayState < 0) // not playing, play plays the selected playlist file
    {
        play(fileIfStopped);
    }
    else
    {
        mpvCommand(QStringList() << QLatin1String("cycle") << QLatin1String("pause"));
    }
}

void MpvController::restart()
{
    seek(0);
    play();
}

void MpvController::rewind()
{
    // if user presses rewind button twice within 3 seconds, stop video
    if (currentTime < 3)
    {
        stop();
    }
    else
    {
        if (currentPlayState == Mpv::Playing)
        {
            restart();
        }
        else
        {
            stop();
        }
    }
}

void MpvController::seek(int pos, bool relative, bool osd)
{
    if (currentPlayState > 0)
    {
        if (relative)
        {
            const QString tmp = ((pos >= 0) ? "+" : QString()) + QString::number(pos);
            if (osd)
            {
                mpvCommand(QStringList() << QLatin1String("osd-msg") << QLatin1String("seek") << tmp);
            }
            else
            {
                mpvCommand(QStringList() << QLatin1String("seek") << tmp);
            }
        }
        else
        {
            if (osd)
            {
                mpvCommand(QVariantList() << QLatin1String("osd-msg") << QLatin1String("seek") << pos << QLatin1String("absolute"));
            }
            else
            {
                mpvCommand(QVariantList() << QLatin1String("seek") << pos << QLatin1String("absolute"));
            }
        }
    }
}

int MpvController::relative(int pos) const
{
    int ret = pos - currentLastTime;
    currentLastTime = pos;
    return ret;
}

void MpvController::screenshot(bool withSubs)
{
    mpvCommand(QStringList() << QLatin1String("screenshot") << QLatin1String(withSubs ? "subtitles" : "video"));
}

void MpvController::frameStep()
{
    mpvCommand(QLatin1String("frame_step"));
}

void MpvController::frameBackStep()
{
    mpvCommand(QLatin1String("frame_back_step"));
}

void MpvController::gotoChapter(int chapter)
{
    mpvSetProperty(QLatin1String("chapter"), chapter);
}

void MpvController::nextChapter()
{
    mpvCommand(QVariantList() << QLatin1String("add") << QLatin1String("chapter") << 1);
}

void MpvController::previousChapter()
{
    mpvCommand(QVariantList() << QLatin1String("add") << QLatin1String("chapter") << -1);
}

void MpvController::addSubtitleTrack(const QString &val)
{
    if (val.isEmpty())
    {
        return;
    }
    mpvCommand(QStringList() << QLatin1String("sub-add") << val);
    // this could be more efficient if we saved tracks in a bst
    auto old = currentFileInfo.tracks; // save the current track-list
    loadTracks(); // load the new track list
    auto current = currentFileInfo.tracks;
    for (auto track : old) // remove the old tracks in current
    {
        current.removeOne(track);
    }
    Mpv::Track &track = current.first();
    emit showText(QString("%0: %1 (%2)").arg(QString::number(track.id), track.title, track.external ? "external" : track.lang));
}

void MpvController::addAudioTrack(const QString &val)
{
    if(val.isEmpty())
    {
        return;
    }
    mpvCommand(QStringList() << QLatin1String("audio-add") << val);
    auto old = currentFileInfo.tracks;
    loadTracks();
    auto current = fileInfo.tracks;
    for (auto track : old)
    {
        current.removeOne(track);
    }
    Mpv::Track &track = current.first();
    emit showText(QString("%0: %1 (%2)").arg(QString::number(track.id), track.title, track.external ? "external" : track.lang));
}

void MpvController::showSubtitles(bool b)
{
    mpvSetProperty(QLatin1String("sub-visibility"), QLatin1String(b ? "yes" : "no"));
}

void MpvController::setMute(bool newMute)
{
    if (currentPlayState > 0)
    {
        mpvSetProperty(QLatin1String("ao-mute"), QLatin1String(newMute ? "yes" : "no"));
    }
    else
    {
        mpvSetOption(QLatin1String("ao-mute"), QLatin1String(newMute ? "yes" : "no"));
    }
    if (currentMute != newMute)
    {
        currentMute = newMute;
        emit muteChanged(currentMute);
    }
}

void MpvController::setHwdec(bool newHwdec, bool osd)
{
    if (currentPlayState > 0)
    {
        mpvSetProperty(QLatin1String("hwdec"), QLatin1String(newHwdec ? "auto" : "no"));
        if (osd)
        {
            emit showText(QString::fromLatin1("%1: %2").arg(tr("Hardware decoding")).arg(newHwdec ? tr("enabled") : tr("disabled")));
        }
    }
    else
    {
        mpvSetOption(QLatin1String("hwdec"), QLatin1String(newHwdec ? "auto" : "no"));
    }
    if (currentHwdec != newHwdec)
    {
        currentHwdec = newHwdec;
        emit hwdecChanged(currentHwdec);
    }
}

void MpvController::setVolume(double vol, bool osd)
{
    if (vol > 100.0)
    {
        vol = 100.0;
    }
    else if (vol < 0.0)
    {
        vol = 0.0;
    }
    if (currentPlayState > 0)
    {
        mpvSetProperty(QLatin1String("ao-volume"), vol);
        if (osd)
        {
            emit showText(tr("Volume: %0%").arg(QString::number(static_cast<int>(vol))));
        }
    }
    else
    {
        mpvSetOption(QLatin1String("volume"), vol);
    }
    if (currentVolume != vol)
    {
        currentVolume = vol;
        emit volumeChanged(currentVolume);
    }
}

void MpvController::setSpeed(double newSpeed)
{
    if (currentPlayState > 0)
    {
        mpvSetProperty(QLatin1String("speed"), newSpeed);
    }
    else
    {
        mpvSetOption(QLatin1String("speed"), newSpeed);
    }
    if (currentSpeed != newSpeed)
    {
        currentSpeed = newSpeed;
        emit speedChanged(currentSpeed);
    }
}

void MpvController::setAspect(const QString &newAspect)
{
    //Examples:
    //--video-aspect=4:3 or --video-aspect=1.3333
    //--video-aspect=16:9 or --video-aspect=1.7777
    //--no-video-aspect or --video-aspect=no
    if (currentPlayState > 0)
    {
        mpvSetProperty(QLatin1String("video-aspect"), newAspect);
    }
    else
    {
        mpvSetOption(QLatin1String("video-aspect"), newAspect);
    }
    if (currentAspect != newAspect)
    {
        currentAspect = newAspect;
        emit aspectChanged(currentAspect);
    }
}

void MpvController::setVid(int val)
{
    if (currentPlayState > 0)
    {
        mpvSetProperty(QLatin1String("vid"), val);
    }
    else
    {
        mpvSetOption(QLatin1String("vid"), val);
    }
    if (currentVid != val)
    {
        currentVid = val;
        emit vidChanged(currentVid);
    }
}

void MpvController::setAid(int val)
{
    if (currentPlayState > 0)
    {
        mpvSetProperty(QLatin1String("aid"), val);
    }
    else
    {
        mpvSetOption(QLatin1String("aid"), val);
    }
    if (currentAid != val)
    {
        currentAid = val;
        emit aidChanged(currentAid);
    }
}

void MpvController::setSid(int val)
{
    if (currentPlayState > 0)
    {
        mpvSetProperty(QLatin1String("sid"), val);
    }
    else
    {
        mpvSetOption(QLatin1String("sid"), val);
    }
    if (currentSid != val)
    {
        currentSid = val;
        emit sidChanged(currentSid);
    }
}

void MpvController::setScreenshotFormat(const QString &val)
{
    if (currentPlayState > 0)
    {
        mpvSetProperty(QLatin1String("screenshot-format"), val);
    }
    else
    {
        mpvSetOption(QLatin1String("screenshot-format"), val);
    }
    if (currentScreenshotFormat != val)
    {
        currentScreenshotFormat = val;
        emit screenshotFormatChanged(currentScreenshotFormat);
    }
}

void MpvController::setScreenshotTemplate(const QString &val)
{
    if (currentPlayState > 0)
    {
        mpvSetProperty(QLatin1String("screenshot-template"), val);
    }
    else
    {
        mpvSetOption(QLatin1String("screenshot-template"), val);
    }
    if (currentScreenshotTemplate != val)
    {
        currentScreenshotTemplate = val;
        emit screenshotTemplateChanged(currentScreenshotTemplate);
    }
}

void MpvController::setScreenshotDirectory(const QString &val)
{
    if (currentPlayState > 0)
    {
        mpvSetProperty(QLatin1String("screenshot-directory"), val);
    }
    else
    {
        mpvSetOption(QLatin1String("screenshot-directory"), val);
    }
    if (currentScreenshotDirectory != val)
    {
        currentScreenshotDirectory = val;
        emit screenshotDirectoryChanged(currentScreenshotDirectory);
    }
}

void MpvController::setSubtitleScale(double scale, bool relative)
{
    mpvCommand(QVariantList() << QLatin1String(relative ? "add" : "set") << QLatin1String("sub-scale") << scale);
}

void MpvController::setDeinterlace(bool b)
{
    if (currentPlayState > 0)
    {
        mpvSetProperty(QLatin1String("deinterlace"), QLatin1String(b ? "yes" : "auto"));
        ShowText(tr("Deinterlacing: %0").arg(b ? tr("enabled") : tr("disabled")));
    }
    else
    {
        mpvSetOption(QLatin1String("deinterlace"), QLatin1String(b ? "yes" : "auto"));
    }
    if (currentDeinterlace != b)
    {
        currentDeinterlace = b;
        emit deinterlaceChanged(currentDeinterlace);
    }
}

void MpvController::setInterpolate(bool b)
{
    if (currentVo.isEmpty())
    {
        currentVo = mpvProperty(QLatin1String("current-vo")).toString();
    }
    QStringList vos = vo.split(',');
    for (QString &o : vos)
    {
        int i = o.indexOf(QLatin1String(":interpolation"));
        if (b && i == -1)
        {
            o.append(QLatin1String(":interpolation"));
        }
        else if (i != -1)
        {
            o.remove(i, QString::fromLatin1(":interpolation").length());
        }
    }
    setVo(vos.join(','));
    emit showText(tr("Motion Interpolation: %0").arg(b ? tr("enabled") : tr("disabled")));
}

void MpvController::setVo(const QString &newVo)
{
    if (currentPlayState > 0)
    {
        mpvSetProperty(QLatin1String("vo"), newVo);
    }
    else
    {
        mpvSetOption(QLatin1String("vo"), newVo);
    }
    if (currentVo != newVo)
    {
        currentVo = newVo;
        emit voChanged(currentVo);
    }
}

void MpvController::setMsgLevel(const QString &newlevel)
{
    mpv_request_log_messages(mpv, newlevel.toUtf8().constData());
    if (currentMsgLevel != newlevel)
    {
        currentMsgLevel = newlevel;
        emit msgLevelChanged(currentMsgLevel);
    }
}

void MpvController::handleMpvEvent(mpv_event *event)
{
    switch (event->event_id)
    {
    case MPV_EVENT_PROPERTY_CHANGE:
    {
        mpv_event_property *prop = (mpv_event_property*)event->data;
        if (QString(prop->name) == QLatin1String("playback-time"))
        {
            if (prop->format == MPV_FORMAT_DOUBLE)
            {
                double newTime = (int)*(double*)prop->data;
                if (currentTime != newTime)
                {
                    currentTime = newTime;
                    currentLastTime = currentTime;
                    emit timeChanged(currentTime);
                }
            }
        }
        else if (QString(prop->name) == QLatin1String("percent-pos"))
        {
            if (prop->format == MPV_FORMAT_DOUBLE)
            {
                double newPercent = (int)*(double*)prop->data;
                if (currentPercent != newPercent)
                {
                    currentPercent = newPercent;
                    emit percentChanged(currentPercent);
                }
            }
        }
        else if (QString(prop->name) == QLatin1String("time-pos"))
        {
            if (prop->format == MPV_FORMAT_DOUBLE)
            {
                double newPosition = (int)*(double*)prop->data;
                if (currentPosition != newPosition)
                {
                    currentPosition = newPosition;
                    emit positionChanged(currentPostion);
                }
            }
        }
        else if (QString(prop->name) == QLatin1String("ao-volume"))
        {
            if (prop->format == MPV_FORMAT_DOUBLE)
            {
                double newVolume = (int)*(double*)prop->data;
                if (currentVolume != newVolume)
                {
                    currentVolume = newVolume;
                    emit volumeChanged(currentVolume);
                }
            }
        }
        else if (QString(prop->name) == QLatin1String("sid"))
        {
            if (prop->format == MPV_FORMAT_INT64)
            {
                int newSid = *(int*)prop->data;
                if (currentSid != newSid)
                {
                    currentSid = newSid;
                    emit sidChanged(currentSid);
                }
            }
        }
        else if (QString(prop->name) == QLatin1String("aid"))
        {
            if (prop->format == MPV_FORMAT_INT64)
            {
                int newAid = *(int*)prop->data;
                if (currentAid != newAid)
                {
                    currentAid = newAid;
                    emit aidChanged(currentAid);
                }
            }
        }
        else if (QString(prop->name) == QLatin1String("sub-visibility"))
        {
            if (prop->format == MPV_FORMAT_FLAG)
            {
                bool newSubtitleVisibility = (bool)*(unsigned*)prop->data;
                if (currentSubtitleVisibility != newSubtitleVisibility)
                {
                    currentSubtitleVisibility = newSubtitleVisibility;
                    emit subtitleVisibilityChanged(currentSubtitleVisibility);
                }
            }
        }
        else if (QString(prop->name) == QLatin1String("ao-mute"))
        {
            if (prop->format == MPV_FORMAT_FLAG)
            {
                bool newMute = (bool)*(unsigned*)prop->data;
                if (currentMute != newMute)
                {
                    currentMute = newMute;
                    emit muteChanged(currentMute);
                }
            }
        }
        else if (QString(prop->name) == QLatin1String("core-idle"))
        {
            if (prop->format == MPV_FORMAT_FLAG)
            {
                if ((bool)*(unsigned*)prop->data && currentPlayState == Mpv::Playing)
                {
                    emit showText(tr("Buffering..."), 0);
                }
                else
                {
                    emit showText(QString(), 0);
                }
            }
        }
        else if (QString(prop->name) == QLatin1String("paused-for-cache"))
        {
            if (prop->format == MPV_FORMAT_FLAG)
            {
                if ((bool)*(unsigned*)prop->data && currentPlayState == Mpv::Playing)
                {
                    emit showText(tr("Your network is slow or stuck, please wait a bit"), 0);
                }
                else
                {
                    emit showText(QString(), 0);
                }
            }
        }
        break;
    }
    case MPV_EVENT_VIDEO_RECONFIG:
    {
        loadFileInfo();
        emit videoReconfig(currentFileInfo.video_params);
        break;
    }
    case MPV_EVENT_AUDIO_RECONFIG:
    {
        loadFileInfo();
        emit audioReconfig(currentFileInfo.audio_params);
        break;
    }
    case MPV_EVENT_IDLE:
        currentFileInfo.duration = 0;
        currentTime = 0;
        emit timeChanged(currentTime);
        currentPlayState = Mpv::Idle;
        emit playStateChanged(currentPlayState);
        break;
    // FIXME: check whether u8sand is right or not.
    // u8sand: these two look like they're reversed but they aren't. the names are misleading.
    // wangwenx190: not sure about this, most other people didn't do it.
    case MPV_EVENT_START_FILE:
        currentPlayState = Mpv::Loaded;
        emit playStateChanged(currentPlayState);
        break;
    case MPV_EVENT_FILE_LOADED:
        emit playbackStarted();
        currentPlayState = Mpv::Started;
        emit playStateChanged(currentPlayState);
        loadFileInfo();
        setVolume(currentVolume);
        setSpeed(currentSpeed);
        setMute(currentMute);
    case MPV_EVENT_UNPAUSE:
        currentPlayState = Mpv::Playing;
        emit playStateChanged(currentPlayState);
        break;
    case MPV_EVENT_PAUSE:
        currentPlayState = Mpv::Paused;
        emit playStateChanged(currentPlayState);
        emit showText(QString(), 0);
        break;
    case MPV_EVENT_END_FILE:
        emit playbackFinished();
        if (currentPlayState == Mpv::Loaded)
        {
            emit showText(tr("File couldn't be opened"));
        }
        currentPlayState = Mpv::Stopped;
        emit playStateChanged(currentPlayState);
        break;
    case MPV_EVENT_SHUTDOWN:
        emit playbackFinished();
        //QCoreApplication::quit();
        break;
    case MPV_EVENT_LOG_MESSAGE:
    {
        mpv_event_log_message *message = static_cast<mpv_event_log_message *>(event->data);
        if (message != nullptr)
        {
            emit mpvMessage(QLatin1String(message->text));
        }
        break;
    }
    default:
        break;
    }
}

static void MpvController::mpvWakeup(void *ctx)
{
    QMetaObject::invokeMethod((MpvController*)ctx, "parseMpvEvents", Qt::QueuedConnection);
}

void MpvController::loadFileInfo()
{
    // load file info
    currentFileInfo.media_title = mpvProperty(QLatin1String("media-title")).toString();
    currentFileInfo.duration = mpvProperty(QLatin1String("duration")).toInt();

    // load tracks
    currentFileInfo.tracks.clear();
    mpv_node tracksNode;
    mpv_get_property(mpv, "track-list", MPV_FORMAT_NODE, &tracksNode);
    currentFileInfo.tracks = mpv::qt::node_to_variant(&tracksNode);
    mpv_free_node_contents(&tracksNode);

    // load chapters
    currentFileInfo.chapters.clear();
    mpv_node chaptersNode;
    mpv_get_property(mpv, "chapter-list", MPV_FORMAT_NODE, &chaptersNode);
    currentFileInfo.chapters = mpv::qt::node_to_variant(&chaptersNode);
    mpv_free_node_contents(&chaptersNode);

    // load video parameters
    currentFileInfo.video_params.codec = mpvProperty(QLatin1String("video-codec")).toString();
    currentFileInfo.video_params.width = mpvProperty(QLatin1String("width")).toInt();
    currentFileInfo.video_params.height = mpvProperty(QLatin1String("height")).toInt();
    currentFileInfo.video_params.dwidth = mpvProperty(QLatin1String("dwidth")).toInt();
    currentFileInfo.video_params.dheight = mpvProperty(QLatin1String("dheight")).toInt();
    currentFileInfo.video_params.aspect = mpvProperty(QLatin1String("video-aspect")).toDouble();

    // load audio parameters
    currentFileInfo.audio_params.codec = mpvProperty(QLatin1String("audio-codec")).toString();
    currentFileInfo.audio_params.params.clear();
    mpv_node AudioNode;
    mpv_get_property(mpv, "audio-params", MPV_FORMAT_NODE, &AudioNode);
    currentFileInfo.audio_params.params = mpv::qt::node_to_variant(&AudioNode);
    mpv_free_node_contents(&AudioNode);

    // load metadata
    currentFileInfo.metadata.clear();
    mpv_node MetaNode;
    mpv_get_property(mpv, "metadata", MPV_FORMAT_NODE, &MetaNode);
    currentFileInfo.metadata = mpv::qt::node_to_variant(&MetaNode);
    mpv_free_node_contents(&MetaNode);

    // load osd size
    currentOsdWidth = mpvProperty(QLatin1String("osd-width")).toInt();
    currentOsdHeight = mpvProperty(QLatin1String("osd-height")).toInt();
}
