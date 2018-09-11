#include <utility>

#include <utility>

#include <utility>

#include <utility>

#include <utility>

#include <utility>

#include <utility>

#include <utility>

#ifndef MPVWIDGET_H
#define MPVWIDGET_H

#include <QOpenGLWidget>

#include <mpv/client.h>
#include <mpv/opengl_cb.h>
#include <mpv/qthelper.hpp>

#include "mpvtypes.h"

class SugoiEngine;

class MpvWidget Q_DECL_FINAL: public QOpenGLWidget
{
friend class SugoiEngine;
    Q_OBJECT
public:
    explicit MpvWidget(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = nullptr, SugoiEngine *se = Q_NULLPTR);
    ~MpvWidget() override;

    const Mpv::FileInfo &getFileInfo()      { return fileInfo; }
    Mpv::PlayState getPlayState()           { return playState; }
    QString getFile()                       { return file; }
    QString getPath()                       { return path; }
    QString getFileFullPath()               { return fileFullPath; }
    QString getScreenshotFormat()           { return screenshotFormat; }
    QString getScreenshotTemplate()         { return screenshotTemplate; }
    QString getScreenshotDir()              { return screenshotDir; }
    QString getVo()                         { return vo; }
    QString getMsgLevel()                   { return msgLevel; }
    double getSpeed()                       { return speed; }
    int getTime()                           { return time; }
    int getVolume()                         { return volume; }
    int getVid()                            { return vid; }
    int getAid()                            { return aid; }
    int getSid()                            { return sid; }
    bool getSubtitleVisibility()            { return subtitleVisibility; }
    bool getMute()                          { return mute; }
    double getPosition()                    { return position; }
    double getDuration()                    { return duration; }
    QSize getVideoSize()                    { return {videoWidth, videoHeight}; }
    bool getHwdec()                         { return hwdec; }
    double getPercent()                     { return percent; }

    int getOsdWidth()                       { return osdWidth; }
    int getOsdHeight()                      { return osdHeight; }

    QString getMediaInfo();

protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;

    bool FileExists(const QString&);

public Q_SLOTS:
    void SetEngine(SugoiEngine *engine);

    void LoadFile(QString);
    QString LoadPlaylist(QString);
    bool PlayFile(const QString&);

    void AddOverlay(int id, int x, int y, const QString& file, int offset, int w, int h);
    void RemoveOverlay(int id);

    void Play();
    void Pause();
    void Stop();
    void PlayPause(const QString& fileIfStopped);
    void Restart();
    void Rewind();
    void Mute(bool);
    void Hwdec(bool h, bool osd = true);

    void Seek(int pos, bool relative = false, bool osd = false);
    int Relative(int pos);
    void FrameStep();
    void FrameBackStep();

    void Chapter(int);
    void NextChapter();
    void PreviousChapter();

    void Volume(int, bool osd = false);
    void Speed(double);
    void Aspect(const QString&);
    void Vid(int);
    void Aid(int);
    void Sid(int);

    void Screenshot(bool withSubs = false);

    void ScreenshotFormat(const QString&);
    void ScreenshotTemplate(const QString&);
    void ScreenshotDirectory(const QString&);

    void AddSubtitleTrack(const QString&);
    void AddAudioTrack(const QString&);
    void ShowSubtitles(bool);
    void SubtitleScale(double scale, bool relative = false);

    void Deinterlace(bool);
    void Interpolate(bool);
    void Vo(QString);

    void MsgLevel(const QString& level);

    void ShowText(const QString& text, int duration = 4000);

    void LoadTracks();
    void LoadChapters();
    void LoadVideoParams();
    void LoadAudioParams();
    void LoadMetadata();
    void LoadOsdSize();

    void command(const QVariant& params);
    void setOption(const QString& name, const QVariant& value);
    void setProperty(const QString& name, const QVariant& value);
    QVariant getProperty(const QString& name) const;

protected Q_SLOTS:
    void OpenFile(const QString&);
    QString PopulatePlaylist();
    void LoadFileInfo();
    void SetProperties();

    void HandleErrorCode(int);

private Q_SLOTS:
    void setPlaylist(const QStringList& l)  { Q_EMIT playlistChanged(l); }
    void setFileInfo()                      { Q_EMIT fileInfoChanged(fileInfo); }
    void setPlayState(Mpv::PlayState s)     { Q_EMIT playStateChanged(playState = s); }
    void setFile(QString s)                 { Q_EMIT fileChanged(file = std::move(s)); }
    void setPath(QString s)                 { Q_EMIT pathChanged(path = std::move(s)); }
    void setFileFullPath(QString s)         { Q_EMIT fileFullPathChanged(fileFullPath = std::move(s)); }
    void setScreenshotFormat(QString s)     { Q_EMIT screenshotFormatChanged(screenshotFormat = std::move(s)); }
    void setScreenshotTemplate(QString s)   { Q_EMIT screenshotTemplateChanged(screenshotTemplate = std::move(s)); }
    void setScreenshotDir(QString s)        { Q_EMIT screenshotDirChanged(screenshotDir = std::move(s)); }
    void setVo(QString s)                   { Q_EMIT voChanged(vo = std::move(s)); }
    void setMsgLevel(QString s)             { Q_EMIT msgLevelChanged(msgLevel = std::move(s)); }
    void setSpeed(double d)                 { Q_EMIT speedChanged(speed = d); }
    void setTime(int i)                     { Q_EMIT timeChanged(time = i); }
    void setVolume(int i)                   { Q_EMIT volumeChanged(volume = i); }
    void setIndex(int i)                    { Q_EMIT indexChanged(index = i); }
    void setVid(int i)                      { Q_EMIT vidChanged(vid = i); }
    void setAid(int i)                      { Q_EMIT aidChanged(aid = i); }
    void setSid(int i)                      { Q_EMIT sidChanged(sid = i); }
    void setSubtitleVisibility(bool b)      { Q_EMIT subtitleVisibilityChanged(subtitleVisibility = b); }
    void setMute(bool b)                    { if (mute != b) Q_EMIT muteChanged(mute = b); }
    void setPosition(double p)              { Q_EMIT positionChanged(position = p); }
    void setDuration(double d)              { Q_EMIT durationChanged(duration = d); }
    void setVideoSize(int w, int h)         { Q_EMIT videoSizeChanged(videoWidth = w, videoHeight = h); }
    void setHwdec(bool b)                   { Q_EMIT hwdecChanged(hwdec = b); }
    void setPercent(double p)               { Q_EMIT percentChanged(percent = p); }

    void swapped();
    void on_mpv_events();
    void maybeUpdate();

Q_SIGNALS:
    void playlistChanged(const QStringList&);
    void fileInfoChanged(const Mpv::FileInfo&);
    void trackListChanged(const QList<Mpv::Track>&);
    void chaptersChanged(const QList<Mpv::Chapter>&);
    void videoParamsChanged(const Mpv::VideoParams&);
    void audioParamsChanged(const Mpv::AudioParams&);
    void playStateChanged(Mpv::PlayState);
    void fileChanging(int, int);
    void fileChanged(QString);
    void pathChanged(QString);
    void fileFullPathChanged(QString);
    void screenshotFormatChanged(QString);
    void screenshotTemplateChanged(QString);
    void screenshotDirChanged(QString);
    void voChanged(QString);
    void msgLevelChanged(QString);
    void speedChanged(double);
    void timeChanged(int);
    void volumeChanged(int);
    void indexChanged(int);
    void vidChanged(int);
    void aidChanged(int);
    void sidChanged(int);
    void debugChanged(bool);
    void subtitleVisibilityChanged(bool);
    void muteChanged(bool);
    void durationChanged(double);
    void positionChanged(double);
    void videoSizeChanged(int, int);
    void hwdecChanged(bool);
    void percentChanged(double);

    void messageSignal(QString m);

private:
    SugoiEngine *sugoi = nullptr;

    // variables
    Mpv::PlayState playState = Mpv::Idle;
    Mpv::FileInfo fileInfo;
    QString     file,
                path,
                fileFullPath,
                screenshotFormat,
                screenshotTemplate,
                screenshotDir,
                suffix,
                vo,
                msgLevel;
    double      speed = 1,
                position = 0,
                duration = 0,
                percent = 0;
    int         time = 0,
                lastTime = 0,
                volume = 100,
                index = 0,
                vid,
                aid,
                sid;
    bool        init = false,
                hwdec = true,
                playlistVisible = false,
                subtitleVisibility = true,
                mute = false;
    int         osdWidth,
                osdHeight,
                videoWidth,
                videoHeight;

private:
    void handle_mpv_event(mpv_event *event);
    static void on_update(void *ctx);

    mpv::qt::Handle mpv;
    mpv_opengl_cb_context *mpv_gl;
};

#endif // MPVWIDGET_H
