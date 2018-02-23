//Hide the console window
#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:\"mainCRTStartup\"")

#include <QCoreApplication>
#include <QProcess>
#include <QFileInfo>
#include <QThread>
#include <QDebug>

static bool run = false;
static QProcess process;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QString filePath = QString();
    if (QFileInfo(QString::fromLatin1("Sugoi64.exe")).exists())
    {
        filePath = QString::fromLatin1("Sugoi64.exe");
    }
    else if (QFileInfo(QString::fromLatin1("Sugoi64d.exe")).exists())
    {
        filePath = QString::fromLatin1("Sugoi64d.exe");
    }
    else if (QFileInfo(QString::fromLatin1("Sugoi.exe")).exists())
    {
        filePath = QString::fromLatin1("Sugoi.exe");
    }
    else if (QFileInfo(QString::fromLatin1("Sugoid.exe")).exists())
    {
        filePath = QString::fromLatin1("Sugoid.exe");
    }
    if (filePath.isEmpty())
    {
        qDebug() << QString::fromLatin1("Main executable not found.");
        return -1;
    }
    filePath += QString::fromLatin1(" --runinbackground");
    run = true;
    while (run)
    {
        process.start(filePath);
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
