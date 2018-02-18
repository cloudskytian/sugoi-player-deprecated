/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Solutions component.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QStringList>
#include <QDir>
#include <QSettings>
#include <QCoreApplication>
#include <QProcess>
#include <QThread>
#include <QFileInfo>

#include "qtservice.h"

class SPlayerService : public QtService<QCoreApplication>
{
public:
    SPlayerService(int argc, char **argv)
    : QtService<QCoreApplication>(argc, argv, QString::fromLatin1("SPlayer Service"))
    {
        setServiceDescription(QString::fromLatin1("SPlayer support service."));
        setServiceFlags(QtServiceBase::CanBeSuspended);
        setStartupType(QtServiceController::AutoStartup);
    }

protected:
    void start()
    {
        QCoreApplication *app = application();
        QString exeName = QString::fromLatin1("SPlayer64.exe");
        QDir curDir(app->applicationDirPath());
        if (QFileInfo(curDir, QString::fromLatin1("SPlayer64.exe")).exists())
        {
            exeName = QString::fromLatin1("SPlayer64.exe");
        }
        else if (QFileInfo(curDir, QString::fromLatin1("SPlayer64d.exe")).exists())
        {
            exeName = QString::fromLatin1("SPlayer64d.exe");
        }
        else if (QFileInfo(curDir, QString::fromLatin1("SPlayer.exe")).exists())
        {
            exeName = QString::fromLatin1("SPlayer.exe");
        }
        else if (QFileInfo(curDir, QString::fromLatin1("SPlayerd.exe")).exists())
        {
            exeName = QString::fromLatin1("SPlayerd.exe");
        }
        else
        {
            logMessage(QString::fromLatin1("SPlayer main executable file not found!"), QtServiceBase::Error);
            app->quit();
        }
        QString exePath = QDir::toNativeSeparators(app->applicationDirPath() + QDir::separator() + exeName);
        for (;;)
        {
            QProcess process;
            process.startDetached(exePath, QStringList() << QString::fromLatin1("--runinbackground"));
            QThread::msleep(15000);
        }
    }

    void stop()
    {
        QCoreApplication *app = application();
        app->quit();
    }
};


int main(int argc, char **argv)
{
#if !defined(Q_OS_WIN)
    // QtService stores service settings in SystemScope, which normally require root privileges.
    // To allow testing this example as non-root, we change the directory of the SystemScope settings file.
    QSettings::setPath(QSettings::NativeFormat, QSettings::SystemScope, QDir::tempPath());
    qWarning("(Example uses dummy settings file: %s/QtSoftware.conf)", QDir::tempPath().toLatin1().constData());
#endif

    SPlayerService service(argc, argv);
    return service.exec();
}
