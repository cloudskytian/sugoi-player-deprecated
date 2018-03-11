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

signals:
    void mpvCommand(const QVariant &params);
    void mpvSetOption(const QString &name, const QVariant &value);
    void mpvSetProperty(const QString &name, const QVariant &value);
    void logoSizeChanged(QSize size);

public slots:
    QString mpvVersion() const;
    MpvController *mpvController() const;
    QWidget *mpvWidget() const;
    QVariant mpvProperty(const QString &name) const;

public slots:
    void setHostWindow(QObject *newHostWindow);
    void setRendererType(Mpv::Renderers newRendererType = Mpv::Renderers::GL);
    void setLogoUrl(const QString &filename);
    void setLogoBackground(const QColor &color);

private:
    Mpv::Renderers currentRendererType = Mpv::Renderers::Null;
    QWidget *currentHostWindow = nullptr;
    MpvController *currentController = nullptr;
    MpvWidgetInterface *currentWidget = nullptr;
    QThread *currentWorker = nullptr;
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
    MpvObject *currentMpvObject = nullptr;
    MpvController *currentController = nullptr;
};
Q_DECLARE_INTERFACE(MpvWidgetInterface, "wangwenx190.SugoiPlayer.MpvWidgetInterface/1.0")

class MpvGLWidget : public QOpenGLWidget, public MpvWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(MpvWidgetInterface)

public:
    explicit MpvGLWidget(MpvObject *object, QWidget *parent = nullptr);
    ~MpvGLWidget();

    QWidget *self();
    void initMpv();
    void setLogoUrl(const QString &filename);
    void setLogoBackground(const QColor &color);
    void setDrawLogo(bool yes);

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);

private:
    static void onMpvUpdate(void *ctx);

private slots:
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
    MpvController(QObject *parent = nullptr);
    ~MpvController();

signals:

public slots:
    void mpvCommand(const QVariant &params);
    void mpvSetOption(const QString &name, const QVariant &value);
    void mpvSetProperty(const QString &name, const QVariant &value);
    QVariant mpvProperty(const QString &name) const;

public slots:
    void create();

    QString clientName() const;
    unsigned long apiVersion() const;
    mpv_opengl_cb_context *mpvDrawContext() const;
    void parseMpvEvents();

private:
    void handleMpvEvent(mpv_event *event);
    static void mpvWakeup(void *ctx);

    mpv::qt::Handle mpv;
    mpv_opengl_cb_context *glMpv = nullptr;
};

#endif // MPVWIDGET_H
