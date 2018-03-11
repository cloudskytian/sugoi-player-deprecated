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
    QWidget *newHostWidget = static_cast<QWidget *>(newHostWindow);
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
        newVBLayout->setContentsMargins(0, 0, 0, 0);
        newVBLayout->setSpacing(0);
        currentHostWindow->setLayout(newVBLayout);
    }
    currentHostWindow->layout()->addWidget(currentWidget->self());
    currentHostWindow->setController(currentController);
    currentHostWindow->initMpv();
}

QString MpvObject::mpvVersion() const
{
    return mpvProperty("mpv-version").toString();
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
    connect(mpvObject, &MpvObject::playbackStarted, this, &MpvGLWidget::self_playbackStarted);
    connect(mpvObject, &MpvObject::playbackFinished, this, &MpvGLWidget::self_playbackFinished);
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

void MpvGLWidget::onMpvUpdate(void *ctx)
{
    QMetaObject::invokeMethod(reinterpret_cast<MpvGLWidget*>(ctx), "maybeUpdate");
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
    //mpv_opengl_cb_set_update_callback(glMpv, MpvWidget::on_update, (void *)this);
    //connect(this, SIGNAL(frameSwapped()), SLOT(swapped()));

    mpv_observe_property(mpv, 0, "duration", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "percent-pos", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "width", MPV_FORMAT_INT64);
    mpv_observe_property(mpv, 0, "height", MPV_FORMAT_INT64);
    mpv_observe_property(mpv, 0, "dwidth", MPV_FORMAT_INT64);
    mpv_observe_property(mpv, 0, "dheight", MPV_FORMAT_INT64);
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
                currentTime = (int)*(double*)prop->data;
                currentLastTime = currentTime;
                emit timeChanged(currentTime);
            }
        }
        else if (QString(prop->name) == QLatin1String("percent-pos"))
        {
            if (prop->format == MPV_FORMAT_DOUBLE)
            {
                currentPercent = (int)*(double*)prop->data;
                emit percentChanged(currentPercent);
            }
        }
        else if (QString(prop->name) == QLatin1String("time-pos"))
        {
            if (prop->format == MPV_FORMAT_DOUBLE)
            {
                currentPosition = (int)*(double*)prop->data;
                emit positionChanged(currentPostion);
            }
        }
        else if (QString(prop->name) == QLatin1String("duration"))
        {
            if (prop->format == MPV_FORMAT_DOUBLE)
            {
                currentDuration = (int)*(double*)prop->data;
                emit durationChanged(currentDuration);
            }
        }
        else if (QString(prop->name) == QLatin1String("ao-volume"))
        {
            if (prop->format == MPV_FORMAT_DOUBLE)
            {
                currentVolume = (int)*(double*)prop->data;
                emit volumeChanged(currentVolume);
            }
        }
        else if (QString(prop->name) == QLatin1String("sid"))
        {
            if (prop->format == MPV_FORMAT_INT64)
            {
                currentSid = *(int*)prop->data;
                emit sidChanged(currentSid);
            }
        }
        else if (QString(prop->name) == QLatin1String("aid"))
        {
            if (prop->format == MPV_FORMAT_INT64)
            {
                currentAid = *(int*)prop->data;
                emit aidChanged(currentAid);
            }
        }
        else if (QString(prop->name) == QLatin1String("sub-visibility"))
        {
            if (prop->format == MPV_FORMAT_FLAG)
            {
                currentSubtitleVisibility = (bool)*(unsigned*)prop->data;
                emit subtitleVisibilityChanged(currentSubtitleVisibility);
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
        int vw = 0, vh = 0;
        vw = mpvProperty(QLatin1String("width")).toInt();
        vh = mpvProperty(QLatin1String("height")).toInt();
        if (vw > 10 && vh > 10)
        {
            if (vw != currentVideoWidth || vh != currentVideoHeight)
            {
                currentVideoWidth = vw;
                currentVideoHeight = vh;
                emit videoSizeChanged(QSize(vw, vh));
            }
        }
        int dw = 0, dh = 0;
        dw = mpvProperty(QLatin1String("dwidth")).toInt();
        dh = mpvProperty(QLatin1String("dheight")).toInt();
        if (dw > 10 && dh > 10)
        {
            if (dw != currentVideoDecodeWidth || dh != currentVideoDecodeHeight)
            {
                currentVideoDecodeWidth = dw;
                currentVideoDecodeHeight = dh;
                emit videoDecodeSizeChanged(QSize(dw, dh));
            }
        }
        break;
    }
    case MPV_EVENT_IDLE:
        currentFileInfo.length = 0;
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
        currentPlayState = Mpv::Started;
        emit playStateChanged(currentPlayState);
        loadFileInfo();
        setProperties();
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
        if (currentPlayState == Mpv::Loaded)
        {
            emit showText(tr("File couldn't be opened"));
        }
        currentPlayState = Mpv::Stopped;
        emit playStateChanged(currentPlayState);
        break;
    case MPV_EVENT_SHUTDOWN:
        QCoreApplication::quit();
        break;
    case MPV_EVENT_LOG_MESSAGE:
    {
        mpv_event_log_message *message = static_cast<mpv_event_log_message*>(event->data);
        if (message != nullptr)
        {
            emit mpvMessage(message->text);
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
