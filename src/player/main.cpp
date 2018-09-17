#include "ui/mainwindow.h"

#include <QFileInfo>
#include <QMimeDatabase>
#include <QDir>
#include <QCommandLineParser>
#include <QApplication>

#include <clocale>

#ifdef Q_OS_WIN
#include <windows.h>
#include "fileassoc.h"
#endif

#include "util.h"
#include "mpvtypes.h"

#ifdef QT_HAS_NETWORK
#include <singleapplication.h>
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

    QApplication::setApplicationName(QStringLiteral("Sugoi Player"));
    QApplication::setApplicationDisplayName(QStringLiteral("Sugoi Player"));
    QApplication::setApplicationVersion(QStringLiteral(SUGOI_VERSION));
    QApplication::setOrganizationName(QStringLiteral("wangwenx190"));
    QApplication::setOrganizationDomain(QStringLiteral("https://wangwenx190.github.io/"));

#ifdef QT_HAS_NETWORK
    SingleApplication instance(argc, argv, true, SingleApplication::Mode::SecondaryNotification);
#else
    QApplication instance(argc, argv);
#endif

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("A free multimedia player for Windows based on libmpv and Qt."));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QStringLiteral("name"), QApplication::translate("main", "The name of the option you want to enable."));
    parser.addPositionalArgument(QStringLiteral("value"), QApplication::translate("main", "The value of the option, if it has."));

#ifdef Q_OS_WIN
    QCommandLineOption regAllOption(QStringLiteral("regall"),
                                        QApplication::translate("main", "Register all media file types."));
    parser.addOption(regAllOption);
    QCommandLineOption regVideoOption(QStringLiteral("regvideo"),
                                      QApplication::translate("main", "Register video media file types."));
    parser.addOption(regVideoOption);
    QCommandLineOption regAudioOption(QStringLiteral("regaudio"),
                                      QApplication::translate("main", "Register audio media file types."));
    parser.addOption(regAudioOption);
    QCommandLineOption unregAllOption(QStringLiteral("unregall"),
                                      QApplication::translate("main", "Unregister all media file types."));
    parser.addOption(unregAllOption);
    QCommandLineOption unregVideoOption(QStringLiteral("unregvideo"),
                                    QApplication::translate("main", "Unregister video media file types."));
    parser.addOption(unregVideoOption);
    QCommandLineOption unregAudioOption(QStringLiteral("unregaudio"),
                                    QApplication::translate("main", "Unregister audio media file types."));
    parser.addOption(unregAudioOption);
#endif
#ifdef QT_HAS_NETWORK
    QCommandLineOption exitOption(QStringLiteral("exit"),
                         QApplication::translate("main", "Terminate all running Sugoi Player instances."));
    parser.addOption(exitOption);
    QCommandLineOption closeOption(QStringLiteral("close"),
                         QApplication::translate("main", "Terminate all running Sugoi Player instances."));
    parser.addOption(closeOption);
    QCommandLineOption quitOption(QStringLiteral("quit"),
                         QApplication::translate("main", "Terminate all running Sugoi Player instances."));
    parser.addOption(quitOption);
    QCommandLineOption newInstanceOption(QStringLiteral("newinstance"),
                                   QApplication::translate("main", "Create a new Sugoi Player instance."));
    parser.addOption(newInstanceOption);
#endif
    QCommandLineOption fileOption(QStringList() << QStringLiteral("f") << QStringLiteral("file"),
                                  QApplication::translate("main",
                                                "Play the given url <url>. It can be a local file or a web url."),
                                  QApplication::translate("main", "url"));
    parser.addOption(fileOption);

    parser.process(instance);    

    QString command = QString();

    bool singleInstance = true;

#ifdef Q_OS_WIN
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
#endif
    if (parser.isSet(newInstanceOption))
    {
        singleInstance = false;
    }
    if (parser.isSet(exitOption) || parser.isSet(closeOption) || parser.isSet(quitOption))
    {
        instance.sendMessage("exit");
        return 0;
    }
    if (parser.isSet(fileOption))
    {
        QString path = parser.value(fileOption);
        command = checkFilePathValidation(path);
    }
    QString path = QApplication::arguments().at(QApplication::arguments().count() - 1);
    command = checkFilePathValidation(path);

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

#ifdef QT_HAS_NETWORK
    if (instance.isSecondary())
    {
#ifdef Q_OS_WIN
        AllowSetForegroundWindow(static_cast<DWORD>(instance.primaryPid()));
#endif
        if (command.isEmpty())
            instance.sendMessage("show");
        else
            instance.sendMessage(command.toUtf8());
        if (singleInstance) return 0;
    }

    if (instance.isPrimary())
    {
        QObject::connect(
                    &instance,
                    &SingleApplication::receivedMessage,
                    [=, &mainWindow](int pid, const QByteArray& msg)
                    {
                        Q_UNUSED(pid)
                        QString message(msg);
                        if (message.isEmpty()) return;
                        if (message == QStringLiteral("show"))
                        {
                            mainWindow.bringToFront();
                            return;
                        }
                        if (message == QStringLiteral("exit")
                             || message == QStringLiteral("quit")
                             || message == QStringLiteral("close"))
                        {
                            mainWindow.close();
                            return;
                        }
                        mainWindow.bringToFront();
                        mainWindow.openFileFromCmd(message);
                     });
    }
#else
    mainWindow.show();
    mainWindow.openFileFromCmd(command);
#endif
    return QApplication::exec();
}
