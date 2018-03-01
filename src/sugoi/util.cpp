#include "util.h"

#include <QTime>
#include <QStringListIterator>
#include <QDir>
#include <QRegExp>
#include <QProcess>
#include <QMimeDatabase>
#include <QDebug>
#include <QStandardPaths>
#include <QMutex>
#include <QObject>
#include <QDateTime>
#include <QApplication>
#include <QSettings>

#include <Windows.h>

#include <stdio.h>
#include <stdlib.h>

namespace Util {

bool IsValidUrl(QString url)
{
    QRegExp rx("^[a-z]{2,}://", Qt::CaseInsensitive); // url
    return (rx.indexIn(url) != -1);
}

QString FormatTime(int _time, int _totalTime)
{
    QTime time = QTime::fromMSecsSinceStartOfDay(_time * 1000);
    if(_totalTime >= 3600) // hours
        return time.toString("h:mm:ss");
    if(_totalTime >= 60)   // minutes
        return time.toString("mm:ss");
    return time.toString("0:ss");   // seconds
}

QString FormatRelativeTime(int _time)
{
    QString prefix;
    if(_time < 0)
    {
        prefix = "-";
        _time = -_time;
    }
    else
        prefix = "+";
    QTime time = QTime::fromMSecsSinceStartOfDay(_time * 1000);
    if(_time >= 3600) // hours
        return prefix+time.toString("h:mm:ss");
    if(_time >= 60)   // minutes
        return prefix+time.toString("mm:ss");
    return prefix+time.toString("0:ss");   // seconds
}

QString FormatNumber(int val, int length)
{
    if(length < 10)
        return QString::number(val);
    else if(length < 100)
        return QString("%1").arg(val, 2, 10, QChar('0'));
    else
        return QString("%1").arg(val, 3, 10, QChar('0'));
}

QString FormatNumberWithAmpersand(int val, int length)
{
    if(length < 10)
        return "&"+QString::number(val);
    else if(length < 100)
    {
        if(val < 10)
            return "0&"+QString::number(val);
        return QString("%1").arg(val, 2, 10, QChar('0'));
    }
    else
    {
        if(val < 10)
            return "00&"+QString::number(val);
        return QString("%1").arg(val, 3, 10, QChar('0'));
    }
}

QString HumanSize(qint64 size)
{
    // taken from http://comments.gmane.org/gmane.comp.lib.qt.general/34914
    float num = size;
    QStringList list;
    list << "KB" << "MB" << "GB" << "TB";

    QStringListIterator i(list);
    QString unit("bytes");

    while(num >= 1024.0 && i.hasNext())
     {
        unit = i.next();
        num /= 1024.0;
    }
    return QString().setNum(num,'f',2)+" "+unit;
}

QString ShortenPathToParent(const Recent &recent)
{
    const int long_name = 100;
    if(recent.title != QString())
        return QString("%0 (%1)").arg(recent.title, recent.path);
    QString p = QDir::fromNativeSeparators(recent.path);
    int i = p.lastIndexOf('/');
    if(i != -1)
    {
        int j = p.lastIndexOf('/', i-1);
        if(j != -1)
        {
            QString parent = p.mid(j+1, i-j-1),
                    file = p.mid(i+1);
            // todo: smarter trimming
            if(parent.length() > long_name)
            {
                parent.truncate(long_name);
                parent += "..";
            }
            if(file.length() > long_name)
            {
                file.truncate(long_name);
                i = p.lastIndexOf('.');
                file += "..";
                if(i != -1)
                {
                    QString ext = p.mid(i);
                    file.truncate(file.length()-ext.length());
                    file += ext; // add the extension back
                }
            }
            return QDir::toNativeSeparators(parent+"/"+file);
        }
    }
    return QDir::toNativeSeparators(recent.path);
}

QStringList ToNativeSeparators(QStringList list)
{
    QStringList ret;
    for(auto element : list)
    {
        if(Util::IsValidLocation(element))
            ret.push_back(element);
        else
            ret.push_back(QDir::toNativeSeparators(element));
    }
    return ret;
}

QStringList FromNativeSeparators(QStringList list)
{
    QStringList ret;
    for(auto element : list)
        ret.push_back(QDir::fromNativeSeparators(element));
    return ret;
}

int GCD(int u, int v)
{
    int shift;
    if(u == 0) return v;
    if(v == 0) return u;
    for(shift = 0; ((u | v) & 1) == 0; ++shift)
    {
       u >>= 1;
       v >>= 1;
    }
    while((u & 1) == 0)
        u >>= 1;
    do
    {
        while ((v & 1) == 0)
            v >>= 1;
        if (u > v)
        {
            unsigned int t = v;
            v = u;
            u = t;
        }
        v = v - u;
    } while (v != 0);
    return u << shift;
}

QString Ratio(int w, int h)
{
    int gcd=GCD(w, h);
    if(gcd == 0)
        return "0:0";
    return QString("%0:%1").arg(QString::number(w/gcd), QString::number(h/gcd));
}

bool DimLightsSupported()
{
    return true;
}

void SetAlwaysOnTop(QMainWindow *window, bool ontop)
{
    window->setWindowFlag(Qt::WindowStaysOnTopHint, ontop);
    window->show();
}

QString SettingsLocation()
{
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + QDir::separator() + QString::fromLatin1("config.ini");
    return QDir::toNativeSeparators(configPath);
}

bool IsValidFile(QString path)
{
    QRegExp rx("^(\\.{1,2}|[a-z]:|\\\\\\\\)", Qt::CaseInsensitive); // relative path, network location, drive
    return (rx.indexIn(path) != -1);
}

bool IsValidLocation(QString loc)
{
    QRegExp rx("^([a-z]{2,}://|\\.{1,2}|[a-z]:|\\\\\\\\)", Qt::CaseInsensitive); // url, relative path, network location, drive
    return (rx.indexIn(loc) != -1);
}

void ShowInFolder(QString path, QString file)
{
    QProcess::startDetached("explorer.exe", QStringList{"/select,", path+file});
}

QString MonospaceFont()
{
    return "Lucida Console";
}

QStringList supportedMimeTypes()
{
    return (QStringList() << QString::fromLatin1("audio/ac3")
            << QString::fromLatin1("audio/eac3")
            << QString::fromLatin1("audio/vnd.dolby.mlp")
            << QString::fromLatin1("audio/vnd.dts")
            << QString::fromLatin1("audio/vnd.dts.hd")
            << QString::fromLatin1("audio/wav")
            << QString::fromLatin1("audio/aiff")
            << QString::fromLatin1("audio/amr")
            << QString::fromLatin1("audio/amr-wb")
            << QString::fromLatin1("audio/basic")
            << QString::fromLatin1("audio/x-ape")
            << QString::fromLatin1("audio/x-wavpack")
            << QString::fromLatin1("audio/x-shorten")
            << QString::fromLatin1("video/vnd.dlna.mpeg-tts")
            << QString::fromLatin1("audio/vnd.dlna.adts")
            << QString::fromLatin1("audio/mpeg")
            << QString::fromLatin1("video/mpeg")
            << QString::fromLatin1("video/dvd")
            << QString::fromLatin1("video/mp4")
            << QString::fromLatin1("audio/mp4")
            << QString::fromLatin1("audio/aac")
            << QString::fromLatin1("audio/flac")
            << QString::fromLatin1("audio/ogg")
            << QString::fromLatin1("video/ogg")
            << QString::fromLatin1("application/ogg")
            << QString::fromLatin1("video/x-matroska")
            << QString::fromLatin1("audio/x-matroska")
            << QString::fromLatin1("video/webm")
            << QString::fromLatin1("audio/webm")
            << QString::fromLatin1("video/avi")
            << QString::fromLatin1("video/x-msvideo")
            << QString::fromLatin1("video/flc")
            << QString::fromLatin1("application/gxf")
            << QString::fromLatin1("application/mxf")
            << QString::fromLatin1("audio/x-ms-wma")
            << QString::fromLatin1("video/x-ms-wm")
            << QString::fromLatin1("video/x-ms-wmv")
            << QString::fromLatin1("video/x-ms-asf")
            << QString::fromLatin1("video/x-flv")
            << QString::fromLatin1("video/mp4")
            << QString::fromLatin1("audio/mp4")
            << QString::fromLatin1("video/quicktime")
            << QString::fromLatin1("application/vnd.rn-realmedia")
            << QString::fromLatin1("application/vnd.rn-realmedia-vbr")
            << QString::fromLatin1("audio/vnd.rn-realaudio")
            << QString::fromLatin1("audio/3gpp")
            << QString::fromLatin1("audio/3gpp2")
            << QString::fromLatin1("video/3gpp")
            << QString::fromLatin1("video/3gpp2")
            << QString::fromLatin1("audio/x-mpegurl")
            << QString::fromLatin1("audio/x-scpls"));
}

QStringList supportedSuffixes()
{
    QStringList mSupportedSuffixes;
    QMimeDatabase mMimeDatabase;
    const QStringList mSupportedMimeTypes = supportedMimeTypes();
    for (const QString &mFileType : mSupportedMimeTypes)
    {
        const QStringList mSuffixes = mMimeDatabase.mimeTypeForName(mFileType).suffixes();
        for (QString mSuffix : mSuffixes)
        {
            mSuffix.prepend(QString::fromLatin1("*."));
            mSupportedSuffixes << mSuffix;
        }
    }
    return mSupportedSuffixes;
}

bool executeProgramWithAdministratorPrivilege(const QString &exePath, const QString &exeParam)
{
    if (exePath.isEmpty())
    {
        qDebug() << "[UAC] : Empty file path.";
        return false;
    }

    SHELLEXECUTEINFO execInfo{sizeof(SHELLEXECUTEINFO)};
    execInfo.lpVerb = TEXT("runas");
    execInfo.lpFile = reinterpret_cast<const wchar_t *>(QDir::toNativeSeparators(exePath).utf16());
    execInfo.nShow = SW_HIDE;
    execInfo.lpParameters = reinterpret_cast<const wchar_t *>(exeParam.utf16());

    if(!ShellExecuteEx(&execInfo))
    {
        DWORD dwStatus = GetLastError();
        if(dwStatus == ERROR_CANCELLED)
        {
            qDebug() << "[UAC] : User denied to give admin privilege.";
        }
        else if(dwStatus == ERROR_FILE_NOT_FOUND)
        {
            qDebug() << "[UAC] : File not found -- " << exePath.toUtf8().constData();
        }
        return false;
    }
    return true;
}

QStringList externalFilesToLoad(const QFile &originalMediaFile, const QString &fileType)
{
    QFileInfo fi(originalMediaFile);
    return externalFilesToLoad(fi, fileType);
}

QStringList externalFilesToLoad(const QFileInfo &originalMediaFile, const QString &fileType)
{
    if (!originalMediaFile.exists() || originalMediaFile.isDir() || fileType.isEmpty())
    {
        return QStringList();
    }
    QDir subDir(originalMediaFile.absoluteDir());
    subDir.setFilter(QDir::Files | QDir::NoSymLinks);
    subDir.setSorting(QDir::Name);
    QFileInfoList fileList = subDir.entryInfoList();
    int fileCount = fileList.count();
    if (fileCount < 2)
    {
        return QStringList();
    }
    const QString fileBaseName = originalMediaFile.baseName().toLower();
    QStringList newFileList;
    for (int i = 0; i < fileCount; ++i)
    {
        QFileInfo fi(fileList.at(i));
        if (fi.absoluteFilePath() == originalMediaFile.absoluteFilePath())
        {
            continue;
        }
        const QString newBaseName = fi.baseName().toLower();
        if (newBaseName == fileBaseName)
        {
            if (fileType.toLower() == QString::fromLatin1("sub"))
            {
                if (fi.suffix().toLower() == QString::fromLatin1("ass")
                        || fi.suffix().toLower() == QString::fromLatin1("ssa")
                        || fi.suffix().toLower() == QString::fromLatin1("srt")
                        || fi.suffix().toLower() == QString::fromLatin1("sup"))
                {
                    newFileList.append(QDir::toNativeSeparators(fi.absoluteFilePath()));
                }
            }
            else if (fileType.toLower() == QString::fromLatin1("audio"))
            {
                if (fi.suffix().toLower() == QString::fromLatin1("mka"))
                {
                    newFileList.append(QDir::toNativeSeparators(fi.absoluteFilePath()));
                }
            }
        }
    }
    return newFileList;
}

QString LogFileLocation()
{
    QString logPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + QDir::separator();
#ifdef _WIN64
    logPath += QString::fromLatin1("debug64.log");
#else
    logPath += QString::fromLatin1("debug.log");
#endif
    return QDir::toNativeSeparators(logPath);
}

void messagesOutputToFile(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    static QMutex mutex;
    mutex.lock();

    QString msgType = QString();
    switch (type)
    {
    case QtDebugMsg:
        msgType = QObject::tr("Debug");
        break;
    case QtInfoMsg:
        msgType = QObject::tr("Information");
        break;
    case QtWarningMsg:
        msgType = QObject::tr("Warning");
        break;
    case QtCriticalMsg:
        msgType = QObject::tr("Critical");
        break;
    case QtFatalMsg:
        msgType = QObject::tr("Fatal");
        break;
    /*case QtSystemMsg:
        msgType = QObject::tr("System");
        break;*/
    default:
        msgType = QObject::tr("Debug");
        break;
    }

    QString dateTimeStr = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd");
    QString messageStr = QString("%1: %2, %3: %4, %5: %6, %7: %8, %9: %10;")
            .arg(msgType.toUtf8().constData())
            .arg(msg.toUtf8().constData())
            .arg(QObject::tr("File"))
            .arg(context.file)
            .arg(QObject::tr("Line"))
            .arg(context.line)
            .arg(QObject::tr("Function"))
            .arg(context.function)
            .arg(QObject::tr("DateTime"))
            .arg(dateTimeStr.toUtf8().constData());

    QFile file(LogFileLocation());
    file.open(QFile::WriteOnly | QFile::Append | QFile::Text);
    QTextStream ts(&file);
    ts << messageStr.toUtf8().constData() << "\r\n";
    file.flush();
    file.close();

    mutex.unlock();

#ifdef _DEBUG
    fprintf_s(stderr, "%s\r\n", messageStr.toUtf8().constData());
#endif
}

bool setAutoStart(const QString &path, const QString &param)
{
#ifdef _DEBUG
    return true;
#endif
    if (path.isEmpty())
    {
        return false;
    }
    const QString key = QString::fromLatin1("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
    QSettings settings(key, QSettings::NativeFormat);
    if (settings.status() != QSettings::NoError)
    {
        return false;
    }
    QString value = QLatin1Char('"') + path + QString::fromLatin1("\" ") + param;
    settings.setValue(QApplication::applicationDisplayName(), QDir::toNativeSeparators(value.trimmed()));
    return true;
}

bool isAutoStart()
{
#ifdef _DEBUG
    return true;
#endif
    const QString key = QString::fromLatin1("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
    QSettings settings(key, QSettings::NativeFormat);
    if (settings.contains(QApplication::applicationDisplayName()))
    {
        return true;
    }
    return false;
}

bool disableAutoStart()
{
#ifdef _DEBUG
    return true;
#endif
    const QString key = QString::fromLatin1("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
    QSettings settings(key, QSettings::NativeFormat);
    if (settings.contains(QApplication::applicationDisplayName()))
    {
        settings.remove(QApplication::applicationDisplayName());
    }
#ifdef _WIN64
    QString cmd = QString::fromLatin1("SugoiGuard64.exe");
#else
    QString cmd = QString::fromLatin1("SugoiGuard.exe");
#endif
    cmd = QString::fromLatin1("TASKKILL /F /IM \"") + cmd + QLatin1Char('"');
    QProcess::execute(cmd);
    return true;
}

}
