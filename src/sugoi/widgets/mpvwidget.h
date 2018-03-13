#ifndef MPVWIDGET_H
#define MPVWIDGET_H

#include <QObject>
#include <QOpenGLWidget>
#include <QColor>

#include <mpv/client.h>
#include <mpv/opengl_cb.h>
#include <mpv/qthelper.hpp>

#include "mpvtypes.h"

class QThread;
class QTimer;
class QWidget;
class MpvWidgetInterface;
class MpvController;
class LogoDrawer;

// FIXME: implement MpvVulkanWidget
typedef MpvGLWidget MpvVulkanWidget;
// FIXME: implement MpvD3DWidget
typedef MpvGLWidget MpvD3DWidget;
// FIXME: implement MpvD2DWidget
typedef MpvGLWidget MpvD2DWidget;
// FIXME: implement MpvGDIWidget
typedef MpvGLWidget MpvGDIWidget;

class MpvObject : public QObject
{
    Q_OBJECT
public:
    explicit MpvObject(QObject *parent = nullptr, const QString &clientName = "mpv");
    ~MpvObject();

private:
    void loadFileInfo();
    void showCursor();
    void hideCursor();

signals:
    void playbackStarted();
    void playbackFinished();
    void mpvCommand(const QVariant &);
    void mpvSetOption(const QString &, const QVariant &);
    void mpvSetProperty(const QString &, const QVariant &);
    void logoSizeChanged(QSize);
    void mouseMoved(int, int);
    void mousePressed(int, int);
    void sugoiCommand(const QString &, const QVariant &);

    void seekableChanged(bool);

    // for properties window
    void playLengthChanged(double);
    void mediaTitleChanged(const QString &);
    void metaDataChanged(const QVariantMap &);
    void chapterDataChanged(const QVariantMap &);
    void chaptersChanged(const QVariantList &);
    void tracksChanged(const QVariantList &);
    void videoSizeChanged(QSize);
    void fileNameChanged(const QString &);
    void fileFormatChanged(const QString &);
    void fileSizeChanged(int64_t);
    void fileCreationTimeChanged(int64_t);
    void filePathChanged(const QString &);

    void playlistChanged(const QStringList &);
    void fileInfoChanged(const Mpv::FileInfo &);
    void videoReconfig(const Mpv::VideoParams &);
    void audioReconfig(const Mpv::AudioParams &);
    void playStateChanged(const Mpv::PlayState &);
    void fileChanging(int, int);
    void fileChanged(const QString &);
    void pathChanged(const QString &);
    void fullPathChanged(const QString &);
    void screenshotFormatChanged(const QString &);
    void screenshotTemplateChanged(const QString &);
    void screenshotDirectoryChanged(const QString &);
    void voChanged(const QString &);
    void msgLevelChanged(const QString &);
    void aspectChanged(const QString &);
    void speedChanged(double);
    void timeChanged(double);
    void volumeChanged(double);
    void indexChanged(int);
    void vidChanged(int);
    void aidChanged(int);
    void sidChanged(int);
    void subtitleVisibilityChanged(bool);
    void muteChanged(bool);
    void deinterlaceChanged(bool);
    void positionChanged(double);
    void hwdecChanged(bool);
    void percentChanged(double);
    void mpvLogMessage(const QString &);
    void showText(const QString &, int duration = 4000);

public slots:
    QString mpvVersion() const;
    MpvController *mpvController() const;
    QWidget *mpvWidget() const;
    QVariant mpvProperty(const QString &name) const;
    QVariant sugoiProperty(const QString &name) const;

    Mpv::FileInfo fileInfo() const;
    Mpv::PlayState playState() const;
    QString file() const;
    QString path() const;
    QString fullPath() const;
    QString screenshotFormat() const;
    QString screenshotTemplate() const;
    QString screenshotDirectory() const;
    QString vo() const;
    QString msgLevel() const;
    double speed() const;
    double time() const;
    double volume() const;
    int vid() const;
    int aid() const;
    int sid() const;
    bool subtitleVisibility() const;
    bool mute() const;
    double position() const;
    bool hwdec() const;
    double percent() const;
    int osdWidth() const;
    int osdHeight() const;
    bool deinterlace() const;
    QString aspect() const;
    QString mediaInfo() const;

public slots:
    void setHostWindow(QObject *newHostWindow);
    void setRendererType(Mpv::Renderers newRendererType = Mpv::Renderers::GL);
    void setLogoUrl(const QString &filename);
    void setLogoBackground(const QColor &color);

public slots:
    void setMouseHideTime(int msec);
    void setMute(bool newMute = false);
    void setHwdec(bool newHwdec = true, bool osd = true);
    void setVolume(double vol = 100.0, bool osd = false);
    void setSpeed(double newSpeed = 1.0);
    void setAspect(const QString &newAspect);
    void setVid(int val = 0);
    void setAid(int val = 0);
    void setSid(int val = 0);
    void setScreenshotFormat(const QString &val);
    void setScreenshotTemplate(const QString &val);
    void setScreenshotDirectory(const QString &val);
    void setSubtitleScale(double scale = 1.0, bool relative = false);
    void setDeinterlace(bool b = true);
    void setInterpolate(bool b = true);
    void setVo(const QString &newVo);
    void setMsgLevel(const QString &newlevel);

public slots:
    void open(const QString &file);
    void load(const QString &file);
    QString loadPlaylist(const QString &file) const;
    bool play(const QString &file) const;
    void play();
    void pause();
    void stop();
    void playPause(const QString &fileIfStopped);
    void restart();
    void rewind();
    void seek(int pos = 0, bool relative = false, bool osd = false);
    int relative(int pos = 0) const;
    void screenshot(bool withSubs = false);
    void frameStep();
    void frameBackStep();
    void gotoChapter(int chapter = 0);
    void nextChapter();
    void previousChapter();
    void addSubtitleTrack(const QString &val);
    void addAudioTrack(const QString &val);
    void showSubtitles(bool b = true);

private slots:
    void handleMpvChangedProperty(const QString &name, const QVariant &v);
    void handleUnhandledMpvEvent(int eventLevel);
    void handleMouseMoved();
    void hideTimerTimeout();
    void self_metadata(const QVariantMap &metadata);

private:
    Mpv::Renderers rendererType = Mpv::Renderers::Null;
    QWidget *hostWindow = nullptr;
    MpvController *controller = nullptr;
    MpvWidgetInterface *widget = nullptr;
    QThread *worker = nullptr;
    QTimer *hideTimer = nullptr;

    Mpv::PlayState currentPlayState = Mpv::Idle;
    Mpv::FileInfo currentFileInfo;
    QString currentFile;
    QString currentPath;
    QString currentFullPath;
    QString currentScreenshotFormat;
    QString currentScreenshotTemplate;
    QString currentScreenshotDirectory;
    QString currentVo;
    QString currentMsgLevel;
    QString currentAspect;
    double currentSpeed = 1.0;
    double currentPosition = 0.0;
    double currentPercent = 0.0;
    double currentTime = 0.0;
    double currentLastTime = 0.0;
    double currentVolume = 100.0;
    int currentVid = 0;
    int currentAid = 0;
    int currentSid = 0;
    bool currentHwdec = true;
    bool currentSubtitleVisibility = true;
    bool currentDeinterlace = true;
    bool currentMute = false;
    bool currentSeekable = false;
    int currentOsdWidth = 0;
    int currentOsdHeight = 0;
};

class MpvWidgetInterface
{
public:
    explicit MpvWidgetInterface(MpvObject *object);
    virtual ~MpvWidgetInterface();

    void setController(MpvController *newController);

    virtual QWidget *self() = 0;
    virtual void initMpv() = 0;
    virtual void setLogoUrl(const QString &filename);
    virtual void setLogoBackground(const QColor &color);
    virtual void setDrawLogo(bool yes);

protected:
    MpvObject *mpvObject = nullptr;
    MpvController *controller = nullptr;
};
Q_DECLARE_INTERFACE(MpvWidgetInterface, "wangwenx190.SugoiPlayer.MpvWidgetInterface/1.0")

class MpvGLWidget Q_DECL_FINAL : public QOpenGLWidget, public MpvWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(MpvWidgetInterface)

public:
    explicit MpvGLWidget(MpvObject *object, QWidget *parent = nullptr);
    ~MpvGLWidget();

protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);

private:
    static void onMpvUpdate(void *ctx);

Q_SIGNALS:
    void mouseMoved(int, int);
    void mousePressed(int, int);

public Q_SLOTS:
    QWidget *self();
    void initMpv();
    void setLogoUrl(const QString &filename);
    void setLogoBackground(const QColor &color);
    void setDrawLogo(bool yes);

private Q_SLOTS:
    void maybeUpdate();
    void self_frameSwapped();
    void self_playbackStarted();
    void self_playbackFinished();

private:
    mpv_opengl_cb_context *glMpv = nullptr;
    bool drawLogo = true;
    LogoDrawer *logo = nullptr;
    int glWidth = 0, glHeight = 0;
};

// This controller attempts to shove as much libmpv related business off of
// the main thread.
class MpvController : public QObject
{
    Q_OBJECT
public:
    struct MpvProperty
    {
        QString name;
        uint64_t userData;
        mpv_format format;
        MpvProperty(const QString &name, uint64_t userData, mpv_format format)
            : name(name), userData(userData), format(format) {}
    };
    typedef QVector<MpvProperty> PropertyList;

    MpvController(QObject *parent = nullptr);
    ~MpvController();

private:
    void setThrottledProperty(const QString &name, const QVariant &v, uint64_t userData);
    void flushProperties();
    void parseMpvEvents();
    void handleMpvEvent(mpv_event *event);
    static void mpvWakeup(void *ctx);
    void handleErrorCode(int error_code);

signals:
    void mpvPropertyChanged(const QString &, const QVariant &, uint64_t);
    void unhandledMpvEvent(int);
    void mpvLogMessage(const QString &);
    void videoReconfig(const Mpv::FileInfo::video_params &);
    void videoSizeChanged(QSize);

public slots:
    void mpvCommand(const QVariant &params);
    void mpvSetOption(const QString &name, const QVariant &value);
    void mpvSetProperty(const QString &name, const QVariant &value);
    QVariant mpvProperty(const QString &name) const;

public slots:
    void create();
    int observeProperties(const MpvController::PropertyList &properties,
                          const QSet<QString> &throttled = QSet<QString>());
    int unobservePropertiesById(const QSet<uint64_t> &ids);
    void setThrottleTime(int msec);

public slots:
    QString clientName() const;
    unsigned long apiVersion() const;
    mpv_opengl_cb_context *mpvDrawContext() const;

private:
    mpv::qt::Handle mpv;
    mpv_opengl_cb_context *glMpv = nullptr;

    QTimer *throttler = nullptr;
    QSet<QString> throttledProperties;
    typedef QMap<QString,QPair<QVariant,uint64_t>> ThrottledValueMap;
    ThrottledValueMap throttledValues;
};

#endif // MPVWIDGET_H
