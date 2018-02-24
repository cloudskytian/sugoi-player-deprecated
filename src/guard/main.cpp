//Hide the console window
#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:\"mainCRTStartup\"")

#include <QCoreApplication>
#include <QProcess>
#include <QFileInfo>
#include <QThread>
#include <QDir>
#include <QDebug>

static bool run = false;
static QProcess process;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
#ifdef _WIN64
    QString filePath = QString::fromLatin1("Sugoi64.exe");
#else
    QString filePath = QString::fromLatin1("Sugoi.exe");
#endif
    filePath = QCoreApplication::applicationDirPath() + QDir::separator() + filePath;
    if (!QFileInfo(filePath).exists())
    {
        qDebug() << QString::fromLatin1("Main executable not found.");
        return -1;
    }
    filePath += QString::fromLatin1(" --runinbackground");
    run = true;
    while (run)
    {
        process.start(QDir::toNativeSeparators(filePath));
        if (!process.waitForFinished())
        {
            run = false;
            qDebug() << QString::fromLatin1("Process error occurred.");
            return -1;
        }
        QThread::msleep(2000);
    }
    return a.exec();
}
