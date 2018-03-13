#include "playbackmanager.h"

#include "util.h"
#include "ui/propertieswindow.h"
#include "ui/mainwindow.h"
#include "widgets/mpvwidget.h"

#include <QWidget>

#ifdef Q_OS_WIN
#include <WinUser.h>
#endif

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

    connect(m_pMainWindow, &MainWindow::destroyed, this, &PlaybackManager::deleteLater);

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
