#ifdef _STATIC_BUILD
#include "sugoilib.h"
#endif

//Hide the console window
#ifndef _STATIC_BUILD
#if !defined(UNICODE) || !defined(_UNICODE)
#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:\"mainCRTStartup\"")
#else
//#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:\"wmainCRTStartup\"")
#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:\"mainCRTStartup\"")
#endif
#endif

#include <QCoreApplication>
#include <QProcess>
#include <QFileInfo>
#include <QThread>
#include <QDir>
#include <QDebug>

#ifdef _STATIC_BUILD
int sugoiGuardMain(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
    QCoreApplication app(argc, argv);
#ifdef _STATIC_BUILD
    QString filePath = QCoreApplication::applicationFilePath();
#else
#ifdef Q_OS_WIN64
    QString filePath = QStringLiteral("Sugoi64.exe");
#else
    QString filePath = QStringLiteral("Sugoi.exe");
#endif
    filePath = QCoreApplication::applicationDirPath() + QDir::separator() + filePath;
    if (!QFileInfo::exists(filePath))
    {
        qDebug() << QStringLiteral("Main executable file not found.");
        return -1;
    }
#endif
    QProcess process;
    while (true)
    {
        process.start(QDir::toNativeSeparators(filePath), QStringList() << QStringLiteral("--runinbackground"));
        process.waitForFinished();
        QThread::msleep(2000);
    }
    return QCoreApplication::exec();
}
