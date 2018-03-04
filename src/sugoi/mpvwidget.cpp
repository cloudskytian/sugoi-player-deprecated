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

#include "sugoiengine.h"
#include "overlayhandler.h"
#include "util.h"

static void wakeup(void *ctx)
{
    QMetaObject::invokeMethod((MpvWidget*)ctx, "on_mpv_events", Qt::QueuedConnection);
}

static void *get_proc_address(void *ctx, const char *name) {
    Q_UNUSED(ctx);
    QOpenGLContext *glctx = QOpenGLContext::currentContext();
    if (!glctx)
        return NULL;
    return (void *)glctx->getProcAddress(QByteArray(name));
}

MpvWidget::MpvWidget(QWidget *parent, Qt::WindowFlags f, SugoiEngine *se) : QOpenGLWidget(parent, f)
{
    sugoi = se;

    mpv = mpv::qt::Handle::FromRawHandle(mpv_create());
    if (!mpv)
    {
        throw std::runtime_error("could not create mpv context");
    }

    setOption(QLatin1String("input-default-bindings"), QLatin1String("no")); // disable mpv default key bindings
    setOption(QLatin1String("input-vo-keyboard"), QLatin1String("no")); // disable keyboard input on the X11 window
    setOption(QLatin1String("input-cursor"), QLatin1String("no")); // no mouse handling
    setOption(QLatin1String("cursor-autohide"), QLatin1String("no")); // no cursor-autohide, we handle that
    setOption(QLatin1String("ytdl"), QLatin1String("yes")); // youtube-dl support

    if (mpv_initialize(mpv) < 0)
    {
        throw std::runtime_error("could not initialize mpv context");
    }

    // Make use of the MPV_SUB_API_OPENGL_CB API.
    setOption(QLatin1String("vo"), QLatin1String("opengl-cb"));

    mpv_gl = (mpv_opengl_cb_context *)mpv_get_sub_api(mpv, MPV_SUB_API_OPENGL_CB);
    if (!mpv_gl)
    {
        throw std::runtime_error("OpenGL not compiled in");
    }
    mpv_opengl_cb_set_update_callback(mpv_gl, MpvWidget::on_update, (void *)this);
    connect(this, SIGNAL(frameSwapped()), SLOT(swapped()));

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

    mpv_set_wakeup_callback(mpv, wakeup, this);
}

MpvWidget::~MpvWidget()
{
    makeCurrent();
    if (mpv_gl)
        mpv_opengl_cb_set_update_callback(mpv_gl, NULL, NULL);
    // Until this call is done, we need to make sure the player remains
    // alive. This is done implicitly with the mpv::qt::Handle instance
    // in this class.
    mpv_opengl_cb_uninit_gl(mpv_gl);
}

void MpvWidget::command(const QVariant& params)
{
    mpv::qt::command_variant(mpv, params);
}

void MpvWidget::setOption(const QString& name, const QVariant& value)
{
    mpv::qt::set_option_variant(mpv, name, value);
}

void MpvWidget::setProperty(const QString& name, const QVariant& value)
{
    mpv::qt::set_property_variant(mpv, name, value);
}

QVariant MpvWidget::getProperty(const QString &name) const
{
    return mpv::qt::get_property_variant(mpv, name);
}

void MpvWidget::initializeGL()
{
    int r = mpv_opengl_cb_init_gl(mpv_gl, NULL, get_proc_address, NULL);
    if (r < 0)
        throw std::runtime_error("could not initialize OpenGL");
}

void MpvWidget::paintGL()
{
    mpv_opengl_cb_draw(mpv_gl, defaultFramebufferObject(), width(), -height());
}

void MpvWidget::swapped()
{
    mpv_opengl_cb_report_flip(mpv_gl, 0);
}

void MpvWidget::on_mpv_events()
{
    // Process all events, until the event queue is empty.
    while (mpv) {
        mpv_event *event = mpv_wait_event(mpv, 0);
        if (event->event_id == MPV_EVENT_NONE) {
            break;
        }
        HandleErrorCode(event->error);
        handle_mpv_event(event);
    }
}

void MpvWidget::handle_mpv_event(mpv_event *event)
{
    switch (event->event_id)
    {
    case MPV_EVENT_PROPERTY_CHANGE:
    {
        mpv_event_property *prop = (mpv_event_property*)event->data;
        if(QString(prop->name) == "playback-time") // playback-time does the same thing as time-pos but works for streaming media
        {
            if(prop->format == MPV_FORMAT_DOUBLE)
            {
                setTime((int)*(double*)prop->data);
                lastTime = time;
            }
        }
        else if(QString(prop->name) == "percent-pos")
        {
            if(prop->format == MPV_FORMAT_DOUBLE)
                setPercent((int)*(double*)prop->data);
        }
        else if(QString(prop->name) == "time-pos")
        {
            if(prop->format == MPV_FORMAT_DOUBLE)
                setPosition((int)*(double*)prop->data);
        }
        else if(QString(prop->name) == "duration")
        {
            if(prop->format == MPV_FORMAT_DOUBLE)
                setDuration((int)*(double*)prop->data);
        }
        else if(QString(prop->name) == "ao-volume")
        {
            if(prop->format == MPV_FORMAT_DOUBLE)
                setVolume((int)*(double*)prop->data);
        }
        else if(QString(prop->name) == "width")
        {
            int w = getProperty(QLatin1String("width")).toInt();
            int h = getProperty(QLatin1String("height")).toInt();
            setVideoSize(w, h);
        }
        else if(QString(prop->name) == "height")
        {
            int w = getProperty(QLatin1String("width")).toInt();
            int h = getProperty(QLatin1String("height")).toInt();
            setVideoSize(w, h);
        }
        else if(QString(prop->name) == "dwidth")
        {
            int dw = getProperty(QLatin1String("dwidth")).toInt();
            int dh = getProperty(QLatin1String("dheight")).toInt();
            setVideoDecodeSize(dw, dh);
        }
        else if(QString(prop->name) == "dheight")
        {
            int dw = getProperty(QLatin1String("dwidth")).toInt();
            int dh = getProperty(QLatin1String("dheight")).toInt();
            setVideoDecodeSize(dw, dh);
        }
        else if(QString(prop->name) == "sid")
        {
            if(prop->format == MPV_FORMAT_INT64)
                setSid(*(int*)prop->data);
        }
        else if(QString(prop->name) == "aid")
        {
            if(prop->format == MPV_FORMAT_INT64)
                setAid(*(int*)prop->data);
        }
        else if(QString(prop->name) == "sub-visibility")
        {
            if(prop->format == MPV_FORMAT_FLAG)
                setSubtitleVisibility((bool)*(unsigned*)prop->data);
        }
        else if(QString(prop->name) == "ao-mute")
        {
            if(prop->format == MPV_FORMAT_FLAG)
                setMute((bool)*(unsigned*)prop->data);
        }
        else if(QString(prop->name) == "core-idle")
        {
            if(prop->format == MPV_FORMAT_FLAG)
            {
                if((bool)*(unsigned*)prop->data && playState == Mpv::Playing)
                    ShowText(tr("Buffering..."), 0);
                else
                    ShowText(QString(), 0);
            }
        }
        else if(QString(prop->name) == "paused-for-cache")
        {
            if(prop->format == MPV_FORMAT_FLAG)
            {
                if((bool)*(unsigned*)prop->data && playState == Mpv::Playing)
                    ShowText(tr("Your network is slow or stuck, please wait a bit"), 0);
                else
                    ShowText(QString(), 0);
            }
        }
        break;
    }
    case MPV_EVENT_IDLE:
        fileInfo.length = 0;
        setTime(0);
        setPlayState(Mpv::Idle);
        break;
        // these two look like they're reversed but they aren't. the names are misleading.
    case MPV_EVENT_START_FILE:
        setPlayState(Mpv::Loaded);
        break;
    case MPV_EVENT_FILE_LOADED:
        setPlayState(Mpv::Started);
        LoadFileInfo();
        SetProperties();
    case MPV_EVENT_UNPAUSE:
        setPlayState(Mpv::Playing);
        break;
    case MPV_EVENT_PAUSE:
        setPlayState(Mpv::Paused);
        ShowText(QString(), 0);
        break;
    case MPV_EVENT_END_FILE:
        if(playState == Mpv::Loaded)
            ShowText(tr("File couldn't be opened"));
        setPlayState(Mpv::Stopped);
        break;
    case MPV_EVENT_SHUTDOWN:
        //QCoreApplication::quit();
        break;
    case MPV_EVENT_LOG_MESSAGE:
    {
        mpv_event_log_message *message = static_cast<mpv_event_log_message*>(event->data);
        if(message != nullptr)
            Q_EMIT messageSignal(message->text);
        break;
    }
    default: // unhandled events
        break;
    }

//    switch (event->event_id) {
//    case MPV_EVENT_PROPERTY_CHANGE: {
//        mpv_event_property *prop = (mpv_event_property *)event->data;
//        if (strcmp(prop->name, "time-pos") == 0) {
//            if (prop->format == MPV_FORMAT_DOUBLE) {
//                double time = *(double *)prop->data;
//                Q_Q_EMIT positionChanged(time);
//            }
//        } else if (strcmp(prop->name, "duration") == 0) {
//            if (prop->format == MPV_FORMAT_DOUBLE) {
//                double time = *(double *)prop->data;
//                Q_Q_EMIT durationChanged(time);
//            }
//        }
//        break;
//    }
//    default: ;
//        // Ignore uninteresting or unknown events.
//    }
}

// Make Qt invoke mpv_opengl_cb_draw() to draw a new/updated video frame.
void MpvWidget::maybeUpdate()
{
    // If the Qt window is not visible, Qt's update() will just skip rendering.
    // This confuses mpv's opengl-cb API, and may lead to small occasional
    // freezes due to video rendering timing out.
    // Handle this by manually redrawing.
    // Note: Qt doesn't seem to provide a way to query whether update() will
    //       be skipped, and the following code still fails when e.g. switching
    //       to a different workspace with a reparenting window manager.
    if (window()->isMinimized()) {
        makeCurrent();
        paintGL();
        context()->swapBuffers(context()->surface());
        swapped();
        doneCurrent();
    } else {
        update();
    }
}

void MpvWidget::on_update(void *ctx)
{
    QMetaObject::invokeMethod((MpvWidget*)ctx, "maybeUpdate");
}

QString MpvWidget::getMediaInfo()
{
    QFileInfo fi(path+file);

    double avsync, fps, vbitrate, abitrate;

    avsync = getProperty(QLatin1String("avsync")).toDouble();
    fps = getProperty(QLatin1String("estimated-vf-fps")).toDouble();
    vbitrate = getProperty(QLatin1String("video-bitrate")).toDouble();
    abitrate = getProperty(QLatin1String("audio-bitrate")).toDouble();
    QString current_vo = getProperty(QLatin1String("current-vo")).toString(),
            current_ao = getProperty(QLatin1String("current-ao")).toString(),
            hwdec_active = getProperty(QLatin1String("hwdec-active")).toString();

    int vtracks = 0,
        atracks = 0;

    for(auto &track : fileInfo.tracks)
    {
        if(track.type == "video")
            ++vtracks;
        else if(track.type == "audio")
            ++atracks;
    }

    const QString outer = "%0: %1\n", inner = "    %0: %1\n";

    QString out = outer.arg(tr("File"), fi.fileName()) +
            inner.arg(tr("Title"), fileInfo.media_title) +
            inner.arg(tr("File size"), Util::HumanSize(fi.size())) +
            inner.arg(tr("Date created"), fi.created().toString()) +
            inner.arg(tr("Media length"), Util::FormatTime(fileInfo.length, fileInfo.length)) + '\n';
    if(fileInfo.video_params.codec != QString())
        out += outer.arg(tr("Video (x%0)").arg(QString::number(vtracks)), fileInfo.video_params.codec) +
            inner.arg(tr("Video Output"), QString("%0 (hwdec %1)").arg(current_vo, hwdec_active)) +
            inner.arg(tr("Resolution"), QString("%0 x %1 (%2)").arg(QString::number(fileInfo.video_params.width),
                                                                    QString::number(fileInfo.video_params.height),
                                                                    Util::Ratio(fileInfo.video_params.width, fileInfo.video_params.height))) +
            inner.arg(tr("FPS"), QString::number(fps)) +
            inner.arg(tr("A/V Sync"), QString::number(avsync)) +
            inner.arg(tr("Bitrate"), tr("%0 kbps").arg(vbitrate/1000)) + '\n';
    if(fileInfo.audio_params.codec != QString())
        out += outer.arg(tr("Audio (x%0)").arg(QString::number(atracks)), fileInfo.audio_params.codec) +
            inner.arg(tr("Audio Output"), current_ao) +
            inner.arg(tr("Sample Rate"), QString::number(fileInfo.audio_params.samplerate)) +
            inner.arg(tr("Channels"), QString::number(fileInfo.audio_params.channels)) +
            inner.arg(tr("Bitrate"), tr("%0 kbps").arg(abitrate)) + '\n';

    if(fileInfo.chapters.length() > 0)
    {
        out += outer.arg(tr("Chapters"), QString());
        int n = 1;
        for(auto &chapter : fileInfo.chapters)
            out += inner.arg(QString::number(n++), chapter.title);
        out += '\n';
    }

    if(fileInfo.metadata.size() > 0)
    {
        out += outer.arg(tr("Metadata"), QString());
        for(auto data = fileInfo.metadata.begin(); data != fileInfo.metadata.end(); ++data)
            out += inner.arg(data.key(), *data);
        out += '\n';
    }

    return out;
}

void MpvWidget::AddOverlay(int id, int x, int y, QString file, int offset, int w, int h)
{
    command(QVariantList() << QLatin1String("overlay_add") << id << x << y << file << offset << QLatin1String("bgra") << w << h << w * 4);
}

void MpvWidget::RemoveOverlay(int id)
{
    command(QVariantList() << QLatin1String("overlay_remove") << id);
}

bool MpvWidget::FileExists(QString f)
{
    if(Util::IsValidUrl(f)) // web url
        return true;
    return QFile(f).exists();
}

void MpvWidget::SetEngine(SugoiEngine *engine)
{
    if (engine == nullptr)
    {
        return;
    }
    sugoi = engine;
}

void MpvWidget::LoadFile(QString f)
{
    PlayFile(LoadPlaylist(f));
}

QString MpvWidget::LoadPlaylist(QString f)
{
    if(f == QString()) // ignore empty file name
        return QString();

    if(f == "-")
    {
        setPath("");
        setPlaylist({f});
    }
    else if(Util::IsValidUrl(f)) // web url
    {
        setPath("");
        setPlaylist({f});
    }
    else // local file
    {
        QFileInfo fi(f);
        if(!fi.exists()) // file doesn't exist
        {
            ShowText(tr("File does not exist")); // tell the user
            return QString(); // don't do anything more
        }
        else if(fi.isDir()) // if directory
        {
            setPath(QDir::toNativeSeparators(fi.absoluteFilePath() + '/')); // set new path
            return PopulatePlaylist();
        }
        else if(fi.isFile()) // if file
        {
            setPath(QDir::toNativeSeparators(fi.absolutePath() + '/')); // set new path
            PopulatePlaylist();
            return fi.fileName();
        }
    }
    return f;
}

bool MpvWidget::PlayFile(QString f)
{
    if(f == QString()) // ignore if file doesn't exist
        return false;

    if(path == QString()) // web url
    {
        OpenFile(f);
        setFile(f);
    }
    else
    {
        QFile qf(path+f);
        if(qf.exists())
        {
            OpenFile(path+f);
            setFile(f);
            setFileFullPath(QDir::toNativeSeparators(path+f));
//            QStringList externalSubList = Util::externalFilesToLoad(qf, QString::fromLatin1("sub"));
//            if (!externalSubList.isEmpty())
//            {
//                for (int i = 0; i < externalSubList.count(); ++i)
//                {
//                    AddSubtitleTrack(externalSubList.at(i));
//                }
//            }
//            QStringList externalAudioList = Util::externalFilesToLoad(qf, QString::fromLatin1("audio"));
//            if (!externalAudioList.isEmpty())
//            {
//                for (int i = 0; i < externalAudioList.count(); ++i)
//                {
//                    AddAudioTrack(externalAudioList.at(i));
//                }
//            }
            Play();
        }
        else
        {
            ShowText(tr("File no longer exists")); // tell the user
            return false;
        }
    }
    return true;
}

void MpvWidget::Play()
{
    if(playState > 0 && mpv)
    {
        setProperty(QLatin1String("pause"), false);
    }
}

void MpvWidget::Pause()
{
    if(playState > 0 && mpv)
    {
        setProperty(QLatin1String("pause"), true);
    }
}

void MpvWidget::Stop()
{
    Restart();
    Pause();
    //command(QLatin1String("stop"));
}

void MpvWidget::PlayPause(QString fileIfStopped)
{
    if(playState < 0) // not playing, play plays the selected playlist file
        PlayFile(fileIfStopped);
    else
    {
        command(QStringList() << QLatin1String("cycle") << QLatin1String("pause"));
    }
}

void MpvWidget::Restart()
{
    Seek(0);
    Play();
}

void MpvWidget::Rewind()
{
    // if user presses rewind button twice within 3 seconds, stop video
    if(time < 3)
    {
        Stop();
    }
    else
    {
        if(playState == Mpv::Playing)
            Restart();
        else
            Stop();
    }
}

void MpvWidget::Mute(bool m)
{
    if(playState > 0)
    {
        setProperty(QLatin1String("ao-mute"), QLatin1String(m ? "yes" : "no"));
    }
    else
        setMute(m);
}

void MpvWidget::Hwdec(bool h, bool osd)
{
    if (playState > 0)
    {
        setProperty(QLatin1String("hwdec"), QLatin1String(h ? "auto" : "no"));
        if(osd)
            ShowText(QString::fromLatin1("%1: %2").arg(tr("Hardware decoding")).arg(tr(h ? "ON" : "OFF")));
    }
    else
    {
        setOption(QLatin1String("hwdec"), QLatin1String(h ? "auto" : "no"));
    }
    setHwdec(h);
}

void MpvWidget::Seek(int pos, bool relative, bool osd)
{
    if(playState > 0)
    {
        if(relative)
        {
            const QString tmp = ((pos >= 0) ? "+" : QString()) + QString::number(pos);
            if(osd)
            {
                command(QStringList() << QLatin1String("osd-msg") << QLatin1String("seek") << tmp);
            }
            else
            {
                command(QStringList() << QLatin1String("seek") << tmp);
            }
        }
        else
        {
            if(osd)
            {
                command(QVariantList() << QLatin1String("osd-msg") << QLatin1String("seek") << pos << QLatin1String("absolute"));
            }
            else
            {
                command(QVariantList() << QLatin1String("seek") << pos << QLatin1String("absolute"));
            }
        }
    }
}

int MpvWidget::Relative(int pos)
{
    int ret = pos - lastTime;
    lastTime = pos;
    return ret;
}

void MpvWidget::FrameStep()
{
    command(QLatin1String("frame_step"));
}

void MpvWidget::FrameBackStep()
{
    command(QLatin1String("frame_back_step"));
}

void MpvWidget::Chapter(int c)
{
    setProperty(QLatin1String("chapter"), c);
}

void MpvWidget::NextChapter()
{
    command(QVariantList() << QLatin1String("add") << QLatin1String("chapter") << 1);
}

void MpvWidget::PreviousChapter()
{
    command(QVariantList() << QLatin1String("add") << QLatin1String("chapter") << -1);
}

void MpvWidget::Volume(int level, bool osd)
{
    if(level > 100) level = 100;
    else if(level < 0) level = 0;

    double v = level;

    if(playState > 0)
    {
        setProperty(QLatin1String("ao-volume"), v);
        if(osd)
            ShowText(tr("Volume: %0%").arg(QString::number(level)));
    }
    else
    {
        setOption(QLatin1String("volume"), v);
        setVolume(level);
    }
}

void MpvWidget::Speed(double d)
{
    if(playState > 0)
        setProperty(QLatin1String("speed"), d);
    setSpeed(d);
}

void MpvWidget::Aspect(QString aspect)
{
    setProperty(QLatin1String("video-aspect"), aspect);
}


void MpvWidget::Vid(int vid)
{
    setProperty(QLatin1String("vid"), vid);
}

void MpvWidget::Aid(int aid)
{
    setProperty(QLatin1String("aid"), aid);
}

void MpvWidget::Sid(int sid)
{
    setProperty(QLatin1String("sid"), sid);
}

void MpvWidget::Screenshot(bool withSubs)
{
    command(QStringList() << QLatin1String("screenshot") << QLatin1String(withSubs ? "subtitles" : "video"));
}

void MpvWidget::ScreenshotFormat(QString s)
{
    setOption(QLatin1String("screenshot-format"), s);
    setScreenshotFormat(s);
}

void MpvWidget::ScreenshotTemplate(QString s)
{
    setOption(QLatin1String("screenshot-template"), s);
    setScreenshotTemplate(s);
}

void MpvWidget::ScreenshotDirectory(QString s)
{
    setOption(QLatin1String("screenshot-directory"), s);
    setScreenshotDir(s);
}

void MpvWidget::AddSubtitleTrack(QString f)
{
    if(f == QString())
        return;
    command(QStringList() << QLatin1String("sub-add") << f);
    // this could be more efficient if we saved tracks in a bst
    auto old = fileInfo.tracks; // save the current track-list
    LoadTracks(); // load the new track list
    auto current = fileInfo.tracks;
    for(auto track : old) // remove the old tracks in current
        current.removeOne(track);
    Mpv::Track &track = current.first();
    ShowText(QString("%0: %1 (%2)").arg(QString::number(track.id), track.title, track.external ? "external" : track.lang));
}

void MpvWidget::AddAudioTrack(QString f)
{
    if(f == QString())
        return;
    command(QStringList() << QLatin1String("audio-add") << f);
    auto old = fileInfo.tracks;
    LoadTracks();
    auto current = fileInfo.tracks;
    for(auto track : old)
        current.removeOne(track);
    Mpv::Track &track = current.first();
    ShowText(QString("%0: %1 (%2)").arg(QString::number(track.id), track.title, track.external ? "external" : track.lang));
}

void MpvWidget::ShowSubtitles(bool b)
{
    setProperty(QLatin1String("sub-visibility"), QLatin1String(b ? "yes" : "no"));
}

void MpvWidget::SubtitleScale(double scale, bool relative)
{
    command(QVariantList() << QLatin1String(relative ? "add" : "set") << QLatin1String("sub-scale") << scale);
}

void MpvWidget::Deinterlace(bool deinterlace)
{
    setProperty(QLatin1String("deinterlace"), QLatin1String(deinterlace ? "yes" : "auto"));
    ShowText(tr("Deinterlacing: %0").arg(deinterlace ? tr("enabled") : tr("disabled")));
}

void MpvWidget::Interpolate(bool interpolate)
{
    if(vo == QString())
        vo = getProperty(QLatin1String("current-vo")).toString();
    QStringList vos = vo.split(',');
    for(auto &o : vos)
    {
        int i = o.indexOf(":interpolation");
        if(interpolate && i == -1)
            o.append(":interpolation");
        else if(i != -1)
            o.remove(i, QString(":interpolation").length());
    }
    setVo(vos.join(','));
    setOption(QLatin1String("vo"), vo);
    ShowText(tr("Motion Interpolation: %0").arg(interpolate ? tr("enabled") : tr("disabled")));
}

void MpvWidget::Vo(QString o)
{
    setVo(o);
    setOption(QLatin1String("vo"), vo);
}

void MpvWidget::MsgLevel(QString level)
{
    QByteArray tmp = level.toUtf8();
    mpv_request_log_messages(mpv, tmp.constData());
    setMsgLevel(level);
}

void MpvWidget::ShowText(QString text, int duration)
{
    if (sugoi == nullptr)
    {
        return;
    }
    sugoi->overlay->showStatusText(text, duration);
}

void MpvWidget::LoadFileInfo()
{
    // get media-title
    fileInfo.media_title = getProperty(QLatin1String("media-title")).toString();
    // get length
    fileInfo.length = getProperty(QLatin1String("duration")).toInt();

    LoadTracks();
    LoadChapters();
    LoadVideoParams();
    LoadAudioParams();
    LoadMetadata();

    Q_EMIT fileInfoChanged(fileInfo);
}

void MpvWidget::LoadTracks()
{
    fileInfo.tracks.clear();
    mpv_node node;
    mpv_get_property(mpv, "track-list", MPV_FORMAT_NODE, &node);
    if(node.format == MPV_FORMAT_NODE_ARRAY)
    {
        for(int i = 0; i < node.u.list->num; i++)
        {
            if(node.u.list->values[i].format == MPV_FORMAT_NODE_MAP)
            {
                Mpv::Track track;
                for(int n = 0; n < node.u.list->values[i].u.list->num; n++)
                {
                    if(QString(node.u.list->values[i].u.list->keys[n]) == "id")
                    {
                        if(node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_INT64)
                            track.id = node.u.list->values[i].u.list->values[n].u.int64;
                    }
                    else if(QString(node.u.list->values[i].u.list->keys[n]) == "type")
                    {
                        if(node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_STRING)
                            track.type = node.u.list->values[i].u.list->values[n].u.string;
                    }
                    else if(QString(node.u.list->values[i].u.list->keys[n]) == "src-id")
                    {
                        if(node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_INT64)
                            track.src_id = node.u.list->values[i].u.list->values[n].u.int64;
                    }
                    else if(QString(node.u.list->values[i].u.list->keys[n]) == "title")
                    {
                        if(node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_STRING)
                            track.title = node.u.list->values[i].u.list->values[n].u.string;
                    }
                    else if(QString(node.u.list->values[i].u.list->keys[n]) == "lang")
                    {
                        if(node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_STRING)
                            track.lang = node.u.list->values[i].u.list->values[n].u.string;
                    }
                    else if(QString(node.u.list->values[i].u.list->keys[n]) == "albumart")
                    {
                        if(node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_FLAG)
                            track.albumart = node.u.list->values[i].u.list->values[n].u.flag;
                    }
                    else if(QString(node.u.list->values[i].u.list->keys[n]) == "default")
                    {
                        if(node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_FLAG)
                            track._default = node.u.list->values[i].u.list->values[n].u.flag;
                    }
                    else if(QString(node.u.list->values[i].u.list->keys[n]) == "external")
                    {
                        if(node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_FLAG)
                            track.external = node.u.list->values[i].u.list->values[n].u.flag;
                    }
                    else if(QString(node.u.list->values[i].u.list->keys[n]) == "external-filename")
                    {
                        if(node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_STRING)
                            track.external_filename = node.u.list->values[i].u.list->values[n].u.string;
                    }
                    else if(QString(node.u.list->values[i].u.list->keys[n]) == "codec")
                    {
                        if(node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_STRING)
                            track.codec = node.u.list->values[i].u.list->values[n].u.string;
                    }
                }
                fileInfo.tracks.push_back(track);
            }
        }
    }

    Q_EMIT trackListChanged(fileInfo.tracks);
}

void MpvWidget::LoadChapters()
{
    fileInfo.chapters.clear();
    mpv_node node;
    mpv_get_property(mpv, "chapter-list", MPV_FORMAT_NODE, &node);
    if(node.format == MPV_FORMAT_NODE_ARRAY)
    {
        for(int i = 0; i < node.u.list->num; i++)
        {
            if(node.u.list->values[i].format == MPV_FORMAT_NODE_MAP)
            {
                Mpv::Chapter ch;
                for(int n = 0; n < node.u.list->values[i].u.list->num; n++)
                {
                    if(QString(node.u.list->values[i].u.list->keys[n]) == "title")
                    {
                        if(node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_STRING)
                            ch.title = node.u.list->values[i].u.list->values[n].u.string;
                    }
                    else if(QString(node.u.list->values[i].u.list->keys[n]) == "time")
                    {
                        if(node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_DOUBLE)
                            ch.time = (int)node.u.list->values[i].u.list->values[n].u.double_;
                    }
                }
                fileInfo.chapters.push_back(ch);
            }
        }
    }
    Q_EMIT chaptersChanged(fileInfo.chapters);
}

void MpvWidget::LoadVideoParams()
{
    fileInfo.video_params.codec = getProperty(QLatin1String("video-codec")).toString();
    fileInfo.video_params.width = getProperty(QLatin1String("width")).toInt();
    fileInfo.video_params.height = getProperty(QLatin1String("height")).toInt();
    fileInfo.video_params.dwidth = getProperty(QLatin1String("dwidth")).toInt();
    fileInfo.video_params.dheight = getProperty(QLatin1String("dheight")).toInt();
    fileInfo.video_params.aspect = getProperty(QLatin1String("video-aspect")).toDouble();

    Q_EMIT videoParamsChanged(fileInfo.video_params);
}

void MpvWidget::LoadAudioParams()
{
    fileInfo.audio_params.codec = getProperty(QLatin1String("audio-codec")).toString();
    mpv_node node;
    mpv_get_property(mpv, "audio-params", MPV_FORMAT_NODE, &node);
    if(node.format == MPV_FORMAT_NODE_MAP)
    {
        for(int i = 0; i < node.u.list->num; i++)
        {
            if(QString(node.u.list->keys[i]) == "samplerate")
            {
                if(node.u.list->values[i].format == MPV_FORMAT_INT64)
                    fileInfo.audio_params.samplerate = node.u.list->values[i].u.int64;
            }
            else if(QString(node.u.list->keys[i]) == "channel-count")
            {
                if(node.u.list->values[i].format == MPV_FORMAT_INT64)
                    fileInfo.audio_params.channels = node.u.list->values[i].u.int64;
            }
        }
    }

    Q_EMIT audioParamsChanged(fileInfo.audio_params);
}

void MpvWidget::LoadMetadata()
{
    fileInfo.metadata.clear();
    mpv_node node;
    mpv_get_property(mpv, "metadata", MPV_FORMAT_NODE, &node);
    if(node.format == MPV_FORMAT_NODE_MAP)
        for(int n = 0; n < node.u.list->num; n++)
            if(node.u.list->values[n].format == MPV_FORMAT_STRING)
                fileInfo.metadata[node.u.list->keys[n]] = node.u.list->values[n].u.string;
}

void MpvWidget::LoadOsdSize()
{
    osdWidth = getProperty(QLatin1String("osd-width")).toInt();
    osdHeight = getProperty(QLatin1String("osd-height")).toInt();
}

void MpvWidget::OpenFile(QString f)
{
    Q_EMIT fileChanging(time, fileInfo.length);

    command(QStringList() << QLatin1String("loadfile") << f);
}

QString MpvWidget::PopulatePlaylist()
{
    if(path != "")
    {
        QStringList playlist;
        QDir root(path);
        QStringList filter = Mpv::media_filetypes;
        if(path != QString() && file != QString())
            filter.append(QString("*.%1").arg(file.split(".").last()));
        QFileInfoList flist;
        flist = root.entryInfoList(filter, QDir::Files);
        for(auto &i : flist)
            playlist.push_back(i.fileName()); // add files to the list
        setPlaylist(playlist);
        if(playlist.empty())
            return QString();
        return playlist.first();
    }
    return QString();
}

void MpvWidget::SetProperties()
{
    Volume(volume);
    Speed(speed);
    Mute(mute);
}

void MpvWidget::HandleErrorCode(int error_code)
{
    if(error_code >= 0)
        return;
    QString error = mpv_error_string(error_code);
    if(error != QString())
        Q_EMIT messageSignal(error+"\n");
}
