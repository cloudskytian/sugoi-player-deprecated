#ifdef _STATIC_BUILD
#include "sugoilib.h"
#endif

#include "ui/mainwindow.h"

#include <QFileInfo>
#include <QMimeDatabase>
#include <QDir>
#include <QCommandLineParser>

#include <locale.h>

#ifdef Q_OS_WIN
#include <Windows.h>
#endif

#include "sugoi-player-version.h"
#include "qtsingleapplication.h"
#include "util.h"
#include "fileassoc.h"

QString checkFilePathValidation(const QString &filePath)
{
    QFileInfo fi(filePath);
    if (fi.exists() && fi.isFile())
    {
        QMimeDatabase mdb;
        if (Util::supportedMimeTypes().contains(mdb.mimeTypeForFile(fi).name()))
        {
            return filePath;
        }
    }
    return QString();
}

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

    QCommandLineParser parser;
    parser.setApplicationDescription(QString::fromStdWString(SUGOI_COMMENTS_STR));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("source", QtSingleApplication::translate("main", "Source file to copy."));
    parser.addPositionalArgument("destination", QtSingleApplication::translate("main", "Destination directory."));

    QCommandLineOption autoStartOption(QString::fromLatin1("autostart"),
                  QtSingleApplication::translate("main", "Make Sugoi Player auto start. Quick start mode only."));
    parser.addOption(autoStartOption);
    QCommandLineOption noAutoStartOption(QString::fromLatin1("noautostart"),
                                      QtSingleApplication::translate("main", "Disable Sugoi Player auto start."));
    parser.addOption(noAutoStartOption);
    QCommandLineOption regAllOption(QString::fromLatin1("regall"),
                                        QtSingleApplication::translate("main", "Register all media file types."));
    parser.addOption(regAllOption);
    QCommandLineOption regVideoOption(QString::fromLatin1("regvideo"),
                                      QtSingleApplication::translate("main", "Register video media file types."));
    parser.addOption(regVideoOption);
    QCommandLineOption regAudioOption(QString::fromLatin1("regaudio"),
                                      QtSingleApplication::translate("main", "Register audio media file types."));
    parser.addOption(regAudioOption);
    QCommandLineOption unregAllOption(QString::fromLatin1("unregall"),
                                      QtSingleApplication::translate("main", "Unregister all media file types."));
    parser.addOption(unregAllOption);
    QCommandLineOption unregVideoOption(QString::fromLatin1("unregvideo"),
                                    QtSingleApplication::translate("main", "Unregister video media file types."));
    parser.addOption(unregVideoOption);
    QCommandLineOption unregAudioOption(QString::fromLatin1("unregaudio"),
                                    QtSingleApplication::translate("main", "Unregister audio media file types."));
    parser.addOption(unregAudioOption);
    QCommandLineOption exitOption(QString::fromLatin1("exit"),
                         QtSingleApplication::translate("main", "Terminate all running Sugoi Player instances."));
    parser.addOption(exitOption);
    QCommandLineOption closeOption(QString::fromLatin1("close"),
                         QtSingleApplication::translate("main", "Terminate all running Sugoi Player instances."));
    parser.addOption(closeOption);
    QCommandLineOption quitOption(QString::fromLatin1("quit"),
                         QtSingleApplication::translate("main", "Terminate all running Sugoi Player instances."));
    parser.addOption(quitOption);
    QCommandLineOption newInstanceOption(QString::fromLatin1("newinstance"),
                                   QtSingleApplication::translate("main", "Create a new Sugoi Player instance."));
    parser.addOption(newInstanceOption);
    QCommandLineOption runInBackgroundOption(QString::fromLatin1("runinbackground"),
                         QtSingleApplication::translate("main",
                             "Run a new instance in background (main window is hidden). Quick start mode only."));
    parser.addOption(runInBackgroundOption);
    QCommandLineOption fileOption(QStringList() << "f" << "file",
                                  QtSingleApplication::translate("main",
                                                "Play the given url <url>. It can be a local file or a web url."),
                                  QtSingleApplication::translate("main", "url"));
    parser.addOption(fileOption);

    parser.process(instance);

    bool singleInstance = true;
    bool runInBackground = false;

    QString command = QString();

    if (parser.isSet(autoStartOption))
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
    else if (parser.isSet(noAutoStartOption))
    {
        if (!Util::disableAutoStart())
        {
            return -1;
        }
        return 0;
    }
    else if (parser.isSet(regAllOption))
    {
        FileAssoc fileAssoc;
        if (!fileAssoc.registerMediaFiles(FileAssoc::reg_type::ALL))
        {
            return -1;
        }
        return 0;
    }
    else if (parser.isSet(regVideoOption))
    {
        FileAssoc fileAssoc;
        if (!fileAssoc.registerMediaFiles(FileAssoc::reg_type::VIDEO_ONLY))
        {
            return -1;
        }
        return 0;
    }
    else if (parser.isSet(regAudioOption))
    {
        FileAssoc fileAssoc;
        if (!fileAssoc.registerMediaFiles(FileAssoc::reg_type::AUDIO_ONLY))
        {
            return -1;
        }
        return 0;
    }
    else if (parser.isSet(unregAllOption))
    {
        FileAssoc fileAssoc;
        fileAssoc.unregisterMediaFiles(FileAssoc::reg_type::ALL);
        return 0;
    }
    else if (parser.isSet(unregVideoOption))
    {
        FileAssoc fileAssoc;
        fileAssoc.unregisterMediaFiles(FileAssoc::reg_type::VIDEO_ONLY);
        return 0;
    }
    else if (parser.isSet(unregAudioOption))
    {
        FileAssoc fileAssoc;
        fileAssoc.unregisterMediaFiles(FileAssoc::reg_type::AUDIO_ONLY);
        return 0;
    }
    else if (parser.isSet(newInstanceOption))
    {
        singleInstance = false;
    }
    else if (parser.isSet(exitOption) || parser.isSet(closeOption) || parser.isSet(quitOption))
    {
        instance.sendMessage(QString::fromLatin1("exit"));
        return 0;
    }
    else if (parser.isSet(runInBackgroundOption))
    {
        runInBackground = true;
        command = QString::fromLatin1("runinbackground");
    }
    else if (parser.isSet(fileOption))
    {
        QString path = parser.value(fileOption);
        command = checkFilePathValidation(path);
    }
    else
    {
        QString path = instance.arguments().at(instance.arguments().count() - 1);
        command = checkFilePathValidation(path);
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
        mainWindow.initMainWindow(false);
        mainWindow.show();
        mainWindow.openFileFromCmd(command);
    }
    else
    {
        mainWindow.initMainWindow(true);
        mainWindow.setWindowOpacity(0.0);
        mainWindow.hide();
        mainWindow.setSysTrayIconVisibility(false);
    }
    SetParent(reinterpret_cast<HWND>(mainWindow.winId()), GetDesktopWindow());

    QObject::connect(&instance, &QtSingleApplication::messageReceived,
                     [=, &mainWindow](const QString &message)
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
                         mainWindow.BringWindowToFront();
                         mainWindow.openFileFromCmd(filePath);
                     });

    HANDLE mutexHandle = CreateMutex(NULL, FALSE
                     , reinterpret_cast<const wchar_t *>(QString::fromStdWString(SUGOI_APP_MUTEX_STR).utf16()));

    int exec = -1;
    exec = instance.exec();

    CloseHandle(mutexHandle);

    return exec;
}
