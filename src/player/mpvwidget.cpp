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
    QMetaObject::invokeMethod(reinterpret_cast<MpvWidget*>(ctx), "on_mpv_events", Qt::QueuedConnection);
}

static void *get_proc_address(void *ctx, const char *name) {
    Q_UNUSED(ctx);
    QOpenGLContext *glctx = QOpenGLContext::currentContext();
    if (!glctx) return nullptr;
    return reinterpret_cast<void *>(glctx->getProcAddress(QByteArray(name)));
}

MpvWidget::MpvWidget(QWidget *parent, Qt::WindowFlags f, SugoiEngine *se) : QOpenGLWidget(parent, f)
{
    sugoi = se;

    mpv = mpv_create();
    if (!mpv) throw std::runtime_error("could not create mpv context");
    if (mpv_initialize(mpv) < 0) throw std::runtime_error("could not initialize mpv context");

    setOption(QStringLiteral("input-default-bindings"), QStringLiteral("no")); // disable mpv default key bindings
    setOption(QStringLiteral("input-vo-keyboard"), QStringLiteral("no")); // disable keyboard input on the X11 window
    setOption(QStringLiteral("input-cursor"), QStringLiteral("no")); // no mouse handling
    setOption(QStringLiteral("cursor-autohide"), QStringLiteral("no")); // no cursor-autohide, we handle that
    setOption(QStringLiteral("ytdl"), QStringLiteral("yes")); // youtube-dl support
    setOption(QStringLiteral("sub-auto"), QStringLiteral("fuzzy")); // Automatic subfile detection

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
    if (mpv_gl) mpv_render_context_free(mpv_gl);
    mpv_terminate_destroy(mpv);
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
    mpv_opengl_init_params gl_init_params{get_proc_address, nullptr, nullptr};
    mpv_render_param params[]{
        {MPV_RENDER_PARAM_API_TYPE, const_cast<char *>(MPV_RENDER_API_TYPE_OPENGL)},
        {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init_params},
        {MPV_RENDER_PARAM_INVALID, nullptr}
    };
    if (mpv_render_context_create(&mpv_gl, mpv, params) < 0)
        throw std::runtime_error("failed to initialize mpv GL context");
    mpv_render_context_set_update_callback(mpv_gl, MpvWidget::on_update, reinterpret_cast<void *>(this));
}

void MpvWidget::paintGL()
{
    mpv_opengl_fbo mpfbo{static_cast<int>(defaultFramebufferObject()), width(), height(), 0};
    int flip_y{1};
    mpv_render_param params[] = {
        {MPV_RENDER_PARAM_OPENGL_FBO, &mpfbo},
        {MPV_RENDER_PARAM_FLIP_Y, &flip_y},
        {MPV_RENDER_PARAM_INVALID, nullptr}
    };
    // See render_gl.h on what OpenGL environment mpv expects, and
    // other API details.
    mpv_render_context_render(mpv_gl, params);
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
        auto *prop = reinterpret_cast<mpv_event_property*>(event->data);
        if(QString(prop->name) == QLatin1String("playback-time")) // playback-time does the same thing as time-pos but works for streaming media
        {
            if(prop->format == MPV_FORMAT_DOUBLE)
            {
                setTime((int)*(double*)prop->data);
                lastTime = time;
            }
        }
        else if(QString(prop->name) == QLatin1String("percent-pos"))
        {
            if(prop->format == MPV_FORMAT_DOUBLE)
                setPercent((int)*(double*)prop->data);
        }
        else if(QString(prop->name) == QLatin1String("time-pos"))
        {
            if(prop->format == MPV_FORMAT_DOUBLE)
                setPosition((int)*(double*)prop->data);
        }
        else if(QString(prop->name) == QLatin1String("duration"))
        {
            if(prop->format == MPV_FORMAT_DOUBLE)
                setDuration((int)*(double*)prop->data);
        }
        else if(QString(prop->name) == QLatin1String("ao-volume"))
        {
            if(prop->format == MPV_FORMAT_DOUBLE)
                setVolume((int)*(double*)prop->data);
        }
        else if(QString(prop->name) == QLatin1String("sid"))
        {
            if(prop->format == MPV_FORMAT_INT64)
                setSid(*(int*)prop->data);
        }
        else if(QString(prop->name) == QLatin1String("aid"))
        {
            if(prop->format == MPV_FORMAT_INT64)
                setAid(*(int*)prop->data);
        }
        else if(QString(prop->name) == QLatin1String("sub-visibility"))
        {
            if(prop->format == MPV_FORMAT_FLAG)
                setSubtitleVisibility((bool)*(unsigned*)prop->data);
        }
        else if(QString(prop->name) == QLatin1String("ao-mute"))
        {
            if(prop->format == MPV_FORMAT_FLAG)
                setMute((bool)*(unsigned*)prop->data);
        }
        else if(QString(prop->name) == QLatin1String("core-idle"))
        {
            if(prop->format == MPV_FORMAT_FLAG)
            {
                if((bool)*(unsigned*)prop->data && playState == Mpv::Playing)
                    ShowText(tr("Buffering..."), 0);
                else
                    ShowText(QString(), 0);
            }
        }
        else if(QString(prop->name) == QLatin1String("paused-for-cache"))
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
    case MPV_EVENT_VIDEO_RECONFIG:
    {
        // Retrieve the new video size.
        int vw = 0, vh = 0;
        vw = getProperty(QStringLiteral("width")).toInt();
        vh = getProperty(QStringLiteral("height")).toInt();
        if (vw > 10 && vh > 10)
        {
            if (vw != videoWidth || vh != videoHeight)
            {
                setVideoSize(vw, vh);
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
        QCoreApplication::quit();
        break;
    case MPV_EVENT_LOG_MESSAGE:
    {
        auto *message = static_cast<mpv_event_log_message*>(event->data);
        if(message != nullptr)
            Q_EMIT messageSignal(message->text);
        break;
    }
    default: // unhandled events
        break;
    }
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
        doneCurrent();
    } else {
        update();
    }
}

void MpvWidget::on_update(void *ctx)
{
    QMetaObject::invokeMethod(reinterpret_cast<MpvWidget*>(ctx), "maybeUpdate");
}

QString MpvWidget::getMediaInfo()
{
    QFileInfo fi(path+file);

    double avsync, fps, vbitrate, abitrate;

    avsync = getProperty(QStringLiteral("avsync")).toDouble();
    fps = getProperty(QStringLiteral("estimated-vf-fps")).toDouble();
    vbitrate = getProperty(QStringLiteral("video-bitrate")).toDouble();
    abitrate = getProperty(QStringLiteral("audio-bitrate")).toDouble();
    QString current_vo = getProperty(QStringLiteral("current-vo")).toString(),
            current_ao = getProperty(QStringLiteral("current-ao")).toString(),
            hwdec_active = getProperty(QStringLiteral("hwdec-active")).toString();

    int vtracks = 0,
        atracks = 0;

    for(auto &track : fileInfo.tracks)
    {
        if(track.type == QLatin1String("video"))
            ++vtracks;
        else if(track.type == QLatin1String("audio"))
            ++atracks;
    }

    const QString outer = QStringLiteral("%0: %1\n"), inner = QStringLiteral("    %0: %1\n");

    QString out = outer.arg(tr("File"), fi.fileName()) +
            inner.arg(tr("Title"), fileInfo.media_title) +
            inner.arg(tr("File size"), Util::HumanSize(fi.size())) +
            inner.arg(tr("Date created"), fi.created().toString()) +
            inner.arg(tr("Media length"), Util::FormatTime(fileInfo.length, fileInfo.length)) + '\n';
    if(fileInfo.video_params.codec != QString())
        out += outer.arg(tr("Video (x%0)").arg(QString::number(vtracks)), fileInfo.video_params.codec) +
            inner.arg(tr("Video Output"), QStringLiteral("%0 (hwdec %1)").arg(current_vo, hwdec_active)) +
            inner.arg(tr("Resolution"), QStringLiteral("%0 x %1 (%2)").arg(QString::number(fileInfo.video_params.width),
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

    if(!fileInfo.metadata.empty())
    {
        out += outer.arg(tr("Metadata"), QString());
        for(auto data = fileInfo.metadata.begin(); data != fileInfo.metadata.end(); ++data)
            out += inner.arg(data.key(), *data);
        out += '\n';
    }

    return out;
}

void MpvWidget::AddOverlay(int id, int x, int y, const QString& file, int offset, int w, int h)
{
    command(QVariantList() << QStringLiteral("overlay_add") << id << x << y << file << offset << QStringLiteral("bgra") << w << h << w * 4);
}

void MpvWidget::RemoveOverlay(int id)
{
    command(QVariantList() << QStringLiteral("overlay_remove") << id);
}

bool MpvWidget::FileExists(const QString& f)
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

void MpvWidget::LoadFile(const QString& f)
{
    PlayFile(LoadPlaylist(f));
}

QString MpvWidget::LoadPlaylist(const QString& f)
{
    if(f == QString()) // ignore empty file name
        return QString();

    if(f == QLatin1String("-"))
    {
        setPath(QLatin1String(""));
        setPlaylist({f});
    }
    else if(Util::IsValidUrl(f)) // web url
    {
        setPath(QLatin1String(""));
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
         if(fi.isDir()) // if directory
        {
            setPath(QDir::toNativeSeparators(fi.absoluteFilePath() + '/')); // set new path
            return PopulatePlaylist();
        }
         if(fi.isFile()) // if file
        {
            setPath(QDir::toNativeSeparators(fi.absolutePath() + '/')); // set new path
            PopulatePlaylist();
            return fi.fileName();
        }
    }
    return f;
}

bool MpvWidget::PlayFile(const QString& f)
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
        setProperty(QStringLiteral("pause"), false);
    }
}

void MpvWidget::Pause()
{
    if(playState > 0 && mpv)
    {
        setProperty(QStringLiteral("pause"), true);
    }
}

void MpvWidget::Stop()
{
    Restart();
    Pause();
    //command(QStringLiteral("stop"));
}

void MpvWidget::PlayPause(const QString& fileIfStopped)
{
    if(playState < 0) // not playing, play plays the selected playlist file
        PlayFile(fileIfStopped);
    else
    {
        command(QStringList() << QStringLiteral("cycle") << QStringLiteral("pause"));
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
        setProperty(QStringLiteral("ao-mute"), m ? "yes" : "no");
    }
    else
        setMute(m);
}

void MpvWidget::Hwdec(bool h, bool osd)
{
    if (playState > 0)
    {
        setProperty(QStringLiteral("hwdec"), h ? "auto" : "no");
        if(osd)
        {
            ShowText(QStringLiteral("%1: %2").arg(tr("Hardware decoding")).arg(h ? tr("enabled") : tr("disabled")));
        }
    }
    else
    {
        setOption(QStringLiteral("hwdec"), h ? "auto" : "no");
    }
    setHwdec(h);
}

void MpvWidget::Seek(int pos, bool relative, bool osd)
{
    if(playState > 0)
    {
        if(relative)
        {
            const QString tmp = ((pos >= 0) ? QStringLiteral("+") : QString()) + QString::number(pos);
            if(osd)
            {
                command(QStringList() << QStringLiteral("osd-msg") << QStringLiteral("seek") << tmp);
            }
            else
            {
                command(QStringList() << QStringLiteral("seek") << tmp);
            }
        }
        else
        {
            if(osd)
            {
                command(QVariantList() << QStringLiteral("osd-msg") << QStringLiteral("seek") << pos << QStringLiteral("absolute"));
            }
            else
            {
                command(QVariantList() << QStringLiteral("seek") << pos << QStringLiteral("absolute"));
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
    command(QStringLiteral("frame_step"));
}

void MpvWidget::FrameBackStep()
{
    command(QStringLiteral("frame_back_step"));
}

void MpvWidget::Chapter(int c)
{
    setProperty(QStringLiteral("chapter"), c);
}

void MpvWidget::NextChapter()
{
    command(QVariantList() << QStringLiteral("add") << QStringLiteral("chapter") << 1);
}

void MpvWidget::PreviousChapter()
{
    command(QVariantList() << QStringLiteral("add") << QStringLiteral("chapter") << -1);
}

void MpvWidget::Volume(int level, bool osd)
{
    if(level > 100) level = 100;
    else if(level < 0) level = 0;

    double v = level;

    if(playState > 0)
    {
        setProperty(QStringLiteral("ao-volume"), v);
        if(osd)
            ShowText(tr("Volume: %0%").arg(QString::number(level)));
    }
    else
    {
        setOption(QStringLiteral("volume"), v);
        setVolume(level);
    }
}

void MpvWidget::Speed(double d)
{
    if(playState > 0)
        setProperty(QStringLiteral("speed"), d);
    setSpeed(d);
}

void MpvWidget::Aspect(const QString& aspect)
{
    setProperty(QStringLiteral("video-aspect"), aspect);
}


void MpvWidget::Vid(int vid)
{
    setProperty(QStringLiteral("vid"), vid);
}

void MpvWidget::Aid(int aid)
{
    setProperty(QStringLiteral("aid"), aid);
}

void MpvWidget::Sid(int sid)
{
    setProperty(QStringLiteral("sid"), sid);
}

void MpvWidget::Screenshot(bool withSubs)
{
    command(QStringList() << QStringLiteral("screenshot") << QLatin1String(withSubs ? "subtitles" : "video"));
}

void MpvWidget::ScreenshotFormat(const QString& s)
{
    setOption(QStringLiteral("screenshot-format"), s);
    setScreenshotFormat(s);
}

void MpvWidget::ScreenshotTemplate(const QString& s)
{
    setOption(QStringLiteral("screenshot-template"), s);
    setScreenshotTemplate(s);
}

void MpvWidget::ScreenshotDirectory(const QString& s)
{
    setOption(QStringLiteral("screenshot-directory"), s);
    setScreenshotDir(s);
}

void MpvWidget::AddSubtitleTrack(const QString& f)
{
    if(f == QString())
        return;
    command(QStringList() << QStringLiteral("sub-add") << f);
    // this could be more efficient if we saved tracks in a bst
    auto old = fileInfo.tracks; // save the current track-list
    LoadTracks(); // load the new track list
    auto current = fileInfo.tracks;
    for(const auto& track : old) // remove the old tracks in current
        current.removeOne(track);
    Mpv::Track &track = current.first();
    ShowText(QStringLiteral("%0: %1 (%2)").arg(QString::number(track.id), track.title, track.external ? QStringLiteral("external") : track.lang));
}

void MpvWidget::AddAudioTrack(const QString& f)
{
    if(f == QString())
        return;
    command(QStringList() << QStringLiteral("audio-add") << f);
    auto old = fileInfo.tracks;
    LoadTracks();
    auto current = fileInfo.tracks;
    for(const auto& track : old)
        current.removeOne(track);
    Mpv::Track &track = current.first();
    ShowText(QStringLiteral("%0: %1 (%2)").arg(QString::number(track.id), track.title, track.external ? QStringLiteral("external") : track.lang));
}

void MpvWidget::ShowSubtitles(bool b)
{
    setProperty(QStringLiteral("sub-visibility"), b ? "yes" : "no");
}

void MpvWidget::SubtitleScale(double scale, bool relative)
{
    command(QVariantList() << QLatin1String(relative ? "add" : "set") << QLatin1String("sub-scale") << scale);
}

void MpvWidget::Deinterlace(bool deinterlace)
{
    setProperty(QStringLiteral("deinterlace"), deinterlace ? "yes" : "auto");
    ShowText(tr("Deinterlacing: %0").arg(deinterlace ? tr("enabled") : tr("disabled")));
}

void MpvWidget::Interpolate(bool interpolate)
{
    if(vo == QString())
        vo = getProperty(QStringLiteral("current-vo")).toString();
    QStringList vos = vo.split(',');
    for(auto &o : vos)
    {
        int i = o.indexOf(QLatin1String(":interpolation"));
        if(interpolate && i == -1)
            o.append(":interpolation");
        else if(i != -1)
            o.remove(i, QStringLiteral(":interpolation").length());
    }
    setVo(vos.join(','));
    setOption(QStringLiteral("vo"), vo);
    ShowText(tr("Motion Interpolation: %0").arg(interpolate ? tr("enabled") : tr("disabled")));
}

void MpvWidget::Vo(const QString& o)
{
    setVo(o);
    setOption(QStringLiteral("vo"), vo);
}

void MpvWidget::MsgLevel(const QString& level)
{
    QByteArray tmp = level.toUtf8();
    mpv_request_log_messages(mpv, tmp.constData());
    setMsgLevel(level);
}

void MpvWidget::ShowText(const QString& text, int duration)
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
    fileInfo.media_title = getProperty(QStringLiteral("media-title")).toString();
    // get length
    fileInfo.length = getProperty(QStringLiteral("duration")).toInt();

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
                    if(QString(node.u.list->values[i].u.list->keys[n]) == QLatin1String("id"))
                    {
                        if(node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_INT64)
                            track.id = node.u.list->values[i].u.list->values[n].u.int64;
                    }
                    else if(QString(node.u.list->values[i].u.list->keys[n]) == QLatin1String("type"))
                    {
                        if(node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_STRING)
                            track.type = node.u.list->values[i].u.list->values[n].u.string;
                    }
                    else if(QString(node.u.list->values[i].u.list->keys[n]) == QLatin1String("src-id"))
                    {
                        if(node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_INT64)
                            track.src_id = node.u.list->values[i].u.list->values[n].u.int64;
                    }
                    else if(QString(node.u.list->values[i].u.list->keys[n]) == QLatin1String("title"))
                    {
                        if(node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_STRING)
                            track.title = node.u.list->values[i].u.list->values[n].u.string;
                    }
                    else if(QString(node.u.list->values[i].u.list->keys[n]) == QLatin1String("lang"))
                    {
                        if(node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_STRING)
                            track.lang = node.u.list->values[i].u.list->values[n].u.string;
                    }
                    else if(QString(node.u.list->values[i].u.list->keys[n]) == QLatin1String("albumart"))
                    {
                        if(node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_FLAG)
                            track.albumart = node.u.list->values[i].u.list->values[n].u.flag;
                    }
                    else if(QString(node.u.list->values[i].u.list->keys[n]) == QLatin1String("default"))
                    {
                        if(node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_FLAG)
                            track._default = node.u.list->values[i].u.list->values[n].u.flag;
                    }
                    else if(QString(node.u.list->values[i].u.list->keys[n]) == QLatin1String("external"))
                    {
                        if(node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_FLAG)
                            track.external = node.u.list->values[i].u.list->values[n].u.flag;
                    }
                    else if(QString(node.u.list->values[i].u.list->keys[n]) == QLatin1String("external-filename"))
                    {
                        if(node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_STRING)
                            track.external_filename = node.u.list->values[i].u.list->values[n].u.string;
                    }
                    else if(QString(node.u.list->values[i].u.list->keys[n]) == QLatin1String("codec"))
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
                    if(QString(node.u.list->values[i].u.list->keys[n]) == QLatin1String("title"))
                    {
                        if(node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_STRING)
                            ch.title = node.u.list->values[i].u.list->values[n].u.string;
                    }
                    else if(QString(node.u.list->values[i].u.list->keys[n]) == QLatin1String("time"))
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
    fileInfo.video_params.codec = getProperty(QStringLiteral("video-codec")).toString();
    fileInfo.video_params.width = getProperty(QStringLiteral("width")).toInt();
    fileInfo.video_params.height = getProperty(QStringLiteral("height")).toInt();
    fileInfo.video_params.dwidth = getProperty(QStringLiteral("dwidth")).toInt();
    fileInfo.video_params.dheight = getProperty(QStringLiteral("dheight")).toInt();
    fileInfo.video_params.aspect = getProperty(QStringLiteral("video-aspect")).toDouble();

    Q_EMIT videoParamsChanged(fileInfo.video_params);
}

void MpvWidget::LoadAudioParams()
{
    fileInfo.audio_params.codec = getProperty(QStringLiteral("audio-codec")).toString();
    mpv_node node;
    mpv_get_property(mpv, "audio-params", MPV_FORMAT_NODE, &node);
    if(node.format == MPV_FORMAT_NODE_MAP)
    {
        for(int i = 0; i < node.u.list->num; i++)
        {
            if(QString(node.u.list->keys[i]) == QLatin1String("samplerate"))
            {
                if(node.u.list->values[i].format == MPV_FORMAT_INT64)
                    fileInfo.audio_params.samplerate = node.u.list->values[i].u.int64;
            }
            else if(QString(node.u.list->keys[i]) == QLatin1String("channel-count"))
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
    osdWidth = getProperty(QStringLiteral("osd-width")).toInt();
    osdHeight = getProperty(QStringLiteral("osd-height")).toInt();
}

void MpvWidget::OpenFile(const QString& f)
{
    Q_EMIT fileChanging(time, fileInfo.length);

    command(QStringList() << QStringLiteral("loadfile") << f);
}

QString MpvWidget::PopulatePlaylist()
{
    if(path != QLatin1String(""))
    {
        QStringList playlist;
        QDir root(path);
        QStringList filter = Mpv::media_filetypes;
        if(path != QString() && file != QString())
            filter.append(QStringLiteral("*.%1").arg(file.split(QStringLiteral(".")).last()));
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
