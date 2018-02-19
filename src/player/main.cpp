#include "ui/mainwindow.h"

#include <QApplication>
#include <QString>
#include <QFileInfo>
#include <QMimeDatabase>

#include <locale.h>

#if defined(Q_OS_WIN)
#include <Windows.h>
#endif

#include "sugoi-player-version.h"
#include "qtsingleapplication.h"
#include "util.h"
#include "winsparkle.h"
#include "fileassoc.h"

int main(int argc, char *argv[])
{
#ifndef _DEBUG
    qInstallMessageHandler(Util::messagesOutputToFile);
#endif

#if defined(Q_OS_WIN)
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
                const QString exePath = QApplication::applicationFilePath();
                const QString exeParam = QString::fromLatin1("--runinbackground");
                if (!Util::setAutoStart(exePath, exeParam))
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

    MainWindow mainWindow;
    if (!runInBackground)
    {
        mainWindow.Load(command, false);
        mainWindow.show();
    }
    else
    {
        mainWindow.setWindowOpacity(0.0);
        mainWindow.Load(QString(), true);
        mainWindow.hide();
    }

    QObject::connect(&instance, &QtSingleApplication::messageReceived,
                     [=, &mainWindow, &instance](const QString &message)
                     {
                         QString filePath(message);
                         if (message == QString::fromLatin1("exit")
                                 || message == QString::fromLatin1("quit")
                                 || message == QString::fromLatin1("close"))
                         {
                             mainWindow.close();
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
                         if (mainWindow.isHidden())
                         {
                             mainWindow.show();
                         }
                         if (mainWindow.windowOpacity() < 1.0)
                         {
                             mainWindow.setWindowOpacity(1.0);
                         }
                         if (instance.activationWindow() != &mainWindow)
                         {
                             instance.setActivationWindow(&mainWindow, true);
                         }
                         if (!mainWindow.isActiveWindow())
                         {
                             instance.activateWindow();
                         }
                         mainWindow.setPauseWhenMinimized(false);
                         mainWindow.openFileFromCmd(filePath);
                         mainWindow.setPauseWhenMinimized(true);
                     });

    HANDLE mutexHandle = CreateMutex(NULL, FALSE
                     , reinterpret_cast<const wchar_t *>(QString::fromStdWString(SUGOI_APP_MUTEX_STR).utf16()));

    win_sparkle_set_appcast_url("https://raw.githubusercontent.com/wangwenx190/Sugoi-Player/master/src/player/appcast.xml");
    //win_sparkle_set_app_details(SUGOI_COMPANY_NAME_STR, SUGOI_APP_NAME_STR, SUGOI_VERSION_STR);
    //win_sparkle_set_automatic_check_for_updates(1);
    win_sparkle_set_lang(mainWindow.getLang().toUtf8().constData());
    win_sparkle_init();
    //win_sparkle_check_update_without_ui();

    int ret = instance.exec();

    win_sparkle_cleanup();

    CloseHandle(mutexHandle);

    return ret;
}
