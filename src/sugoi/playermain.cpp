#ifdef _STATIC_BUILD
#include "sugoilib.h"
#endif

#include "ui/mainwindow.h"

#include <QFileInfo>
#include <QMimeDatabase>
#include <QDir>
#include <QSystemTrayIcon>

#include <locale.h>

#ifdef Q_OS_WIN
#include <Windows.h>
#endif

#include "sugoi-player-version.h"
#include "qtsingleapplication.h"
#include "util.h"
#include "fileassoc.h"

#ifdef _STATIC_BUILD
int sugoiAppMain(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
    qInstallMessageHandler(Util::messagesOutputToFile);

#ifdef Q_OS_WIN
    FreeConsole();
#endif

    QtSingleApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QtSingleApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QtSingleApplication::setApplicationName(QString::fromStdWString(SUGOI_APP_NAME_STR));
    QtSingleApplication::setApplicationDisplayName(QString::fromStdWString(SUGOI_APP_DISPLAY_NAME_STR));
    QtSingleApplication::setApplicationVersion(QString::fromStdWString(SUGOI_VERSION_STR));
    QtSingleApplication::setOrganizationName(QString::fromStdWString(SUGOI_COMPANY_NAME_STR));
    QtSingleApplication::setOrganizationDomain(QString::fromStdWString(SUGOI_COMPANY_URL_STR));

    QtSingleApplication instance(QString::fromStdWString(SUGOI_APP_MUTEX_STR), argc, argv);

    bool singleInstance = true;
    bool runInBackground = false;

    QStringList cmdline = instance.arguments();
    QString command = QString();

    if (cmdline.count() > 1)
    {
        for (int i = 1; i <= (cmdline.count() - 1); ++i)
        {
            if (cmdline.at(i) == QString::fromLatin1("--autostart"))
            {
#ifdef _STATIC_BUILD
                QString filePath = QtSingleApplication::applicationFilePath();
                QString fileParam = QString::fromLatin1("--guard");
#else
#ifdef Q_OS_WIN64
                QString filePath = QString::fromLatin1("SugoiGuard64.exe");
#else
                QString filePath = QString::fromLatin1("SugoiGuard.exe");
#endif
                filePath = QtSingleApplication::applicationDirPath() + QDir::separator() + filePath;
                if (!QFileInfo(filePath).exists())
                {
                    return -1;
                }
                QString fileParam = QString();
#endif
                if (!Util::setAutoStart(QDir::toNativeSeparators(filePath), fileParam))
                {
                    return -1;
                }
                return 0;
            }
            else if (cmdline.at(i) == QString::fromLatin1("--noautostart"))
            {
                if (!Util::disableAutoStart())
                {
                    return -1;
                }
                return 0;
            }
            else if (cmdline.at(i) == QString::fromLatin1("--regall"))
            {
                FileAssoc fileAssoc;
                if (!fileAssoc.registerMediaFiles(FileAssoc::reg_type::ALL))
                {
                    return -1;
                }
                return 0;
            }
            else if (cmdline.at(i) == QString::fromLatin1("--regvideo"))
            {
                FileAssoc fileAssoc;
                if (!fileAssoc.registerMediaFiles(FileAssoc::reg_type::VIDEO_ONLY))
                {
                    return -1;
                }
                return 0;
            }
            else if (cmdline.at(i) == QString::fromLatin1("--regaudio"))
            {
                FileAssoc fileAssoc;
                if (!fileAssoc.registerMediaFiles(FileAssoc::reg_type::AUDIO_ONLY))
                {
                    return -1;
                }
                return 0;
            }
            else if (cmdline.at(i) == QString::fromLatin1("--unregall"))
            {
                FileAssoc fileAssoc;
                fileAssoc.unregisterMediaFiles(FileAssoc::reg_type::ALL);
                return 0;
            }
            else if (cmdline.at(i) == QString::fromLatin1("--unregvideo"))
            {
                FileAssoc fileAssoc;
                fileAssoc.unregisterMediaFiles(FileAssoc::reg_type::VIDEO_ONLY);
                return 0;
            }
            else if (cmdline.at(i) == QString::fromLatin1("--unregaudio"))
            {
                FileAssoc fileAssoc;
                fileAssoc.unregisterMediaFiles(FileAssoc::reg_type::AUDIO_ONLY);
                return 0;
            }
            else if (cmdline.at(i) == QString::fromLatin1("--newinstance"))
            {
                singleInstance = false;
            }
            else if (cmdline.at(i) == QString::fromLatin1("--exit")
                     || cmdline.at(i) == QString::fromLatin1("--quit")
                     || cmdline.at(i) == QString::fromLatin1("--close"))
            {
                instance.sendMessage(QString::fromLatin1("exit"));
                return 0;
            }
            else if (cmdline.at(i) == QString::fromLatin1("--runinbackground"))
            {
                runInBackground = true;
                command = QString::fromLatin1("runinbackground");
                break;
            }
            else
            {
                QFileInfo fi(cmdline.at(i));
                if (fi.exists() && fi.isFile())
                {
                    QMimeDatabase mdb;
                    if (Util::supportedMimeTypes().contains(mdb.mimeTypeForFile(fi).name()))
                    {
                        command = cmdline.at(i);
                        break;
                    }
                }
            }
        }
    }

    if (singleInstance)
    {
        if (instance.sendMessage(command))
        {
            return 0;
        }
    }

    // Qt sets the locale in the QApplication constructor, but libmpv requires
    // the LC_NUMERIC category to be set to "C", so change it back.
    setlocale(LC_NUMERIC, "C");

    MainWindow *mainWindow = nullptr;
    if (!runInBackground)
    {
        mainWindow = new MainWindow(nullptr, false);
        SetParent(reinterpret_cast<HWND>(mainWindow->winId()), GetDesktopWindow());
        mainWindow->show();
        mainWindow->openFileFromCmd(command);
    }
    else
    {
        mainWindow = new MainWindow(nullptr, true);
        SetParent(reinterpret_cast<HWND>(mainWindow->winId()), GetDesktopWindow());
        mainWindow->setWindowOpacity(0.0);
        mainWindow->hide();
        if (mainWindow->getSystemTrayIcon() != nullptr)
        {
            mainWindow->getSystemTrayIcon()->hide();
        }
    }

    QObject::connect(&instance, &QtSingleApplication::messageReceived,
                     [=, &mainWindow, &instance](const QString &message)
                     {
                         QString filePath(message);
                         if (message == QString::fromLatin1("exit")
                                 || message == QString::fromLatin1("quit")
                                 || message == QString::fromLatin1("close"))
                         {
                             mainWindow->close();
                             return;
                         }
                         else if (message == QString::fromLatin1("runinbackground"))
                         {
                             if (runInBackground)
                             {
                                 return;
                             }
                             else
                             {
                                 filePath = QString();
                             }
                         }
                         mainWindow->BringWindowToFront();
                         mainWindow->openFileFromCmd(filePath);
                     });

    HANDLE mutexHandle = CreateMutex(NULL, FALSE
                     , reinterpret_cast<const wchar_t *>(QString::fromStdWString(SUGOI_APP_MUTEX_STR).utf16()));

    int exec = instance.exec();

    delete mainWindow;
    mainWindow = nullptr;

    CloseHandle(mutexHandle);

    return exec;
}
