#include "ui/mainwindow.h"

#include <QFileInfo>
#include <QMimeDatabase>
#include <QDir>
#include <QCommandLineParser>

#include <clocale>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#include "sugoi-player-version.h"
#include "util.h"
#include "fileassoc.h"
#include "mpvtypes.h"

#ifdef QT_HAS_NETWORK
#include <SingleApplication.h>
#define SugoiApplication SingleApplication
#else
#define SugoiApplication QApplication
#endif

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

int main(int argc, char *argv[])
{
    qInstallMessageHandler(Util::messagesOutputToFile);

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    SugoiApplication::setApplicationName(QString::fromStdWString(SUGOI_APP_NAME_STR));
    SugoiApplication::setApplicationDisplayName(QString::fromStdWString(SUGOI_APP_DISPLAY_NAME_STR));
    SugoiApplication::setApplicationVersion(QString::fromStdWString(SUGOI_VERSION_STR));
    SugoiApplication::setOrganizationName(QString::fromStdWString(SUGOI_COMPANY_NAME_STR));
    SugoiApplication::setOrganizationDomain(QString::fromStdWString(SUGOI_COMPANY_URL_STR));

#ifdef QT_HAS_NETWORK
    SugoiApplication instance(argc, argv, true, SugoiApplication::Mode::SecondaryNotification);
#else
    SugoiApplication instance(argc, argv);
#endif

    QCommandLineParser parser;
    parser.setApplicationDescription(QString::fromStdWString(SUGOI_COMMENTS_STR));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QStringLiteral("name"), SugoiApplication::translate("main", "The name of the option you want to enable."));
    parser.addPositionalArgument(QStringLiteral("value"), SugoiApplication::translate("main", "The value of the option, if it has."));

    QCommandLineOption autoStartOption(QStringLiteral("autostart"),
                  SugoiApplication::translate("main", "Make Sugoi Player auto start. Quick start mode only."));
    parser.addOption(autoStartOption);
    QCommandLineOption noAutoStartOption(QStringLiteral("noautostart"),
                                      SugoiApplication::translate("main", "Disable Sugoi Player auto start."));
    parser.addOption(noAutoStartOption);
    QCommandLineOption regAllOption(QStringLiteral("regall"),
                                        SugoiApplication::translate("main", "Register all media file types."));
    parser.addOption(regAllOption);
    QCommandLineOption regVideoOption(QStringLiteral("regvideo"),
                                      SugoiApplication::translate("main", "Register video media file types."));
    parser.addOption(regVideoOption);
    QCommandLineOption regAudioOption(QStringLiteral("regaudio"),
                                      SugoiApplication::translate("main", "Register audio media file types."));
    parser.addOption(regAudioOption);
    QCommandLineOption unregAllOption(QStringLiteral("unregall"),
                                      SugoiApplication::translate("main", "Unregister all media file types."));
    parser.addOption(unregAllOption);
    QCommandLineOption unregVideoOption(QStringLiteral("unregvideo"),
                                    SugoiApplication::translate("main", "Unregister video media file types."));
    parser.addOption(unregVideoOption);
    QCommandLineOption unregAudioOption(QStringLiteral("unregaudio"),
                                    SugoiApplication::translate("main", "Unregister audio media file types."));
    parser.addOption(unregAudioOption);
    QCommandLineOption exitOption(QStringLiteral("exit"),
                         SugoiApplication::translate("main", "Terminate all running Sugoi Player instances."));
    parser.addOption(exitOption);
    QCommandLineOption closeOption(QStringLiteral("close"),
                         SugoiApplication::translate("main", "Terminate all running Sugoi Player instances."));
    parser.addOption(closeOption);
    QCommandLineOption quitOption(QStringLiteral("quit"),
                         SugoiApplication::translate("main", "Terminate all running Sugoi Player instances."));
    parser.addOption(quitOption);
    QCommandLineOption newInstanceOption(QStringLiteral("newinstance"),
                                   SugoiApplication::translate("main", "Create a new Sugoi Player instance."));
    parser.addOption(newInstanceOption);
    QCommandLineOption fileOption(QStringList() << QStringLiteral("f") << QStringLiteral("file"),
                                  SugoiApplication::translate("main",
                                                "Play the given url <url>. It can be a local file or a web url."),
                                  SugoiApplication::translate("main", "url"));
    parser.addOption(fileOption);

    parser.process(instance);

    bool singleInstance = true;

    QString command = QString();

    if (parser.isSet(autoStartOption))
    {
#ifdef Q_OS_WIN64
        QString filePath = QStringLiteral("SugoiGuard64.exe");
#else
        QString filePath = QStringLiteral("SugoiGuard.exe");
#endif
        filePath = SugoiApplication::applicationDirPath() + QDir::separator() + filePath;
        if (!QFileInfo::exists(filePath))
        {
            return -1;
        }
        QString fileParam = QString();
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
    if (parser.isSet(fileOption))
    {
        QString path = parser.value(fileOption);
        command = checkFilePathValidation(path);
    }
    QString path = SugoiApplication::arguments().at(SugoiApplication::arguments().count() - 1);
    command = checkFilePathValidation(path);

    if (singleInstance)
    {
        instance.sendMessage(command.toUtf8());
        return 0;
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
    mainWindow.initMainWindow();
    mainWindow.show();
    mainWindow.openFileFromCmd(command);

#ifdef QT_HAS_NETWORK
    if (instance.isSecondary())
    {
        AllowSetForegroundWindow(static_cast<DWORD>(instance.primaryPid()));
        instance.sendMessage(command.toUtf8());
        return 0;
    }
    if (instance.isPrimary())
    {
        QObject::connect(
        &instance,
        &SugoiApplication::receivedMessage,
        [=, &mainWindow](int pid, QByteArray msg)
        {
            Q_UNUSED(pid)
            QString message(msg);
            if (message.isEmpty()) return;
            if (message == QStringLiteral("exit")
                    || message == QStringLiteral("quit")
                    || message == QStringLiteral("close"))
            {
                mainWindow.close();
                return;
            }
            mainWindow.raise();
            mainWindow.activateWindow();
            mainWindow.openFileFromCmd(message);
        });
    }
#endif

    HANDLE mutexHandle = CreateMutex(NULL, FALSE
                     , reinterpret_cast<const wchar_t *>(QString::fromStdWString(SUGOI_APP_MUTEX_STR).utf16()));

    int exec = -1;
    exec = SugoiApplication::exec();

    CloseHandle(mutexHandle);

    return exec;
}
