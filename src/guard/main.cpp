//Hide the console window
#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:\"mainCRTStartup\"")

#include <QCoreApplication>
#include <QProcess>
#include <QFileInfo>
#include <QThread>
#include <QDir>
#include <QDebug>

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
    QProcess process;
    while (true)
    {
        process.start(QDir::toNativeSeparators(filePath), QStringList() << QString::fromLatin1("--runinbackground"));
        process.waitForFinished();
        QThread::msleep(2000);
    }
    return a.exec();
}
