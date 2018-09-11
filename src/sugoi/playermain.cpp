#ifdef _STATIC_BUILD
#include "sugoilib.h"
#endif

#include "ui/mainwindow.h"

#include <QFileInfo>
#include <QMimeDatabase>
#include <QDir>
#include <QCommandLineParser>

#include <clocale>

#ifdef Q_OS_WIN
#include <Windows.h>
#endif

#include "sugoi-player-version.h"
#include "qtsingleapplication.h"
#include "util.h"
#include "fileassoc.h"
#include "mpvtypes.h"

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

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QCoreApplication::setApplicationName(QString::fromStdWString(SUGOI_APP_NAME_STR));
    QCoreApplication::setApplicationDisplayName(QString::fromStdWString(SUGOI_APP_DISPLAY_NAME_STR));
    QCoreApplication::setApplicationVersion(QString::fromStdWString(SUGOI_VERSION_STR));
    QCoreApplication::setOrganizationName(QString::fromStdWString(SUGOI_COMPANY_NAME_STR));
    QCoreApplication::setOrganizationDomain(QString::fromStdWString(SUGOI_COMPANY_URL_STR));

    QtSingleApplication instance(QString::fromStdWString(SUGOI_APP_MUTEX_STR), argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription(QString::fromStdWString(SUGOI_COMMENTS_STR));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QStringLiteral("name"), QtSingleApplication::translate("main", "The name of the option you want to enable."));
    parser.addPositionalArgument(QStringLiteral("value"), QtSingleApplication::translate("main", "The value of the option, if it has."));

    QCommandLineOption autoStartOption(QStringLiteral("autostart"),
                  QtSingleApplication::translate("main", "Make Sugoi Player auto start. Quick start mode only."));
    parser.addOption(autoStartOption);
    QCommandLineOption noAutoStartOption(QStringLiteral("noautostart"),
                                      QtSingleApplication::translate("main", "Disable Sugoi Player auto start."));
    parser.addOption(noAutoStartOption);
    QCommandLineOption regAllOption(QStringLiteral("regall"),
                                        QtSingleApplication::translate("main", "Register all media file types."));
    parser.addOption(regAllOption);
    QCommandLineOption regVideoOption(QStringLiteral("regvideo"),
                                      QtSingleApplication::translate("main", "Register video media file types."));
    parser.addOption(regVideoOption);
    QCommandLineOption regAudioOption(QStringLiteral("regaudio"),
                                      QtSingleApplication::translate("main", "Register audio media file types."));
    parser.addOption(regAudioOption);
    QCommandLineOption unregAllOption(QStringLiteral("unregall"),
                                      QtSingleApplication::translate("main", "Unregister all media file types."));
    parser.addOption(unregAllOption);
    QCommandLineOption unregVideoOption(QStringLiteral("unregvideo"),
                                    QtSingleApplication::translate("main", "Unregister video media file types."));
    parser.addOption(unregVideoOption);
    QCommandLineOption unregAudioOption(QStringLiteral("unregaudio"),
                                    QtSingleApplication::translate("main", "Unregister audio media file types."));
    parser.addOption(unregAudioOption);
    QCommandLineOption exitOption(QStringLiteral("exit"),
                         QtSingleApplication::translate("main", "Terminate all running Sugoi Player instances."));
    parser.addOption(exitOption);
    QCommandLineOption closeOption(QStringLiteral("close"),
                         QtSingleApplication::translate("main", "Terminate all running Sugoi Player instances."));
    parser.addOption(closeOption);
    QCommandLineOption quitOption(QStringLiteral("quit"),
                         QtSingleApplication::translate("main", "Terminate all running Sugoi Player instances."));
    parser.addOption(quitOption);
    QCommandLineOption newInstanceOption(QStringLiteral("newinstance"),
                                   QtSingleApplication::translate("main", "Create a new Sugoi Player instance."));
    parser.addOption(newInstanceOption);
    QCommandLineOption runInBackgroundOption(QStringLiteral("runinbackground"),
                         QtSingleApplication::translate("main",
                             "Run a new instance in background (main window is hidden). Quick start mode only."));
    parser.addOption(runInBackgroundOption);
    QCommandLineOption fileOption(QStringList() << QStringLiteral("f") << QStringLiteral("file"),
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
        QString fileParam = QStringLiteral("--guard");
#else
#ifdef Q_OS_WIN64
        QString filePath = QStringLiteral("SugoiGuard64.exe");
#else
        QString filePath = QStringLiteral("SugoiGuard.exe");
#endif
        filePath = QtSingleApplication::applicationDirPath() + QDir::separator() + filePath;
        if (!QFileInfo::exists(filePath))
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
    if (parser.isSet(noAutoStartOption))
    {
        if (!Util::disableAutoStart())
        {
            return -1;
        }
        return 0;
    }
    if (parser.isSet(regAllOption))
    {
        FileAssoc fileAssoc;
        if (!fileAssoc.registerMediaFiles(FileAssoc::reg_type::ALL))
        {
            return -1;
        }
        return 0;
    }
    if (parser.isSet(regVideoOption))
    {
        FileAssoc fileAssoc;
        if (!fileAssoc.registerMediaFiles(FileAssoc::reg_type::VIDEO_ONLY))
        {
            return -1;
        }
        return 0;
    }
    if (parser.isSet(regAudioOption))
    {
        FileAssoc fileAssoc;
        if (!fileAssoc.registerMediaFiles(FileAssoc::reg_type::AUDIO_ONLY))
        {
            return -1;
        }
        return 0;
    }
    if (parser.isSet(unregAllOption))
    {
        FileAssoc fileAssoc;
        fileAssoc.unregisterMediaFiles(FileAssoc::reg_type::ALL);
        return 0;
    }
    if (parser.isSet(unregVideoOption))
    {
        FileAssoc fileAssoc;
        fileAssoc.unregisterMediaFiles(FileAssoc::reg_type::VIDEO_ONLY);
        return 0;
    }
    if (parser.isSet(unregAudioOption))
    {
        FileAssoc fileAssoc;
        fileAssoc.unregisterMediaFiles(FileAssoc::reg_type::AUDIO_ONLY);
        return 0;
    }
    if (parser.isSet(newInstanceOption))
    {
        singleInstance = false;
    }
    if (parser.isSet(exitOption) || parser.isSet(closeOption) || parser.isSet(quitOption))
    {
        instance.sendMessage(QStringLiteral("exit"));
        return 0;
    }
    if (parser.isSet(runInBackgroundOption))
    {
        runInBackground = true;
        command = QStringLiteral("runinbackground");
    }
    if (parser.isSet(fileOption))
    {
        QString path = parser.value(fileOption);
        command = checkFilePathValidation(path);
    }
    QString path = QtSingleApplication::arguments().at(QtSingleApplication::arguments().count() - 1);
    command = checkFilePathValidation(path);

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

    qRegisterMetaType<Mpv::PlayState>();
    qRegisterMetaType<Mpv::Chapter>();
    qRegisterMetaType<Mpv::Track>();
    qRegisterMetaType<Mpv::VideoParams>();
    qRegisterMetaType<Mpv::AudioParams>();
    qRegisterMetaType<Mpv::FileInfo>();

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
                         if (message == QStringLiteral("exit")
                                 || message == QStringLiteral("quit")
                                 || message == QStringLiteral("close"))
                         {
                             mainWindow.close();
                             return;
                         }
                         if (message == QStringLiteral("runinbackground"))
                         {
                             if (runInBackground)
                             {
                                 return;
                             }
                             filePath = QString();
                         }
                         mainWindow.BringWindowToFront();
                         mainWindow.openFileFromCmd(filePath);
                     });

    HANDLE mutexHandle = CreateMutex(NULL, FALSE
                     , reinterpret_cast<const wchar_t *>(QString::fromStdWString(SUGOI_APP_MUTEX_STR).utf16()));

    int exec = -1;
    exec = QtSingleApplication::exec();

    CloseHandle(mutexHandle);

    return exec;
}
