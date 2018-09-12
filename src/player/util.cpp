#include "util.h"

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
#include <QTime>
#include <QWidget>
//#include <QFile>
//#include <QFileInfo>

#include <windows.h>

namespace Util {

void SetAlwaysOnTop(QWidget *window, bool ontop)
{
    window->setWindowFlag(Qt::WindowStaysOnTopHint, ontop);
    window->show();
}

QString SettingsLocation()
{
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + QDir::separator() + QStringLiteral("config.ini");
    return QDir::toNativeSeparators(configPath);
}

void ShowInFolder(const QString& path, const QString& file)
{
    QProcess::startDetached(QStringLiteral("explorer.exe"), QStringList{"/select,", path+file});
}

QStringList supportedMimeTypes()
{
    return (QStringList() << QStringLiteral("audio/ac3")
            << QStringLiteral("audio/eac3")
            << QStringLiteral("audio/vnd.dolby.mlp")
            << QStringLiteral("audio/vnd.dts")
            << QStringLiteral("audio/vnd.dts.hd")
            << QStringLiteral("audio/wav")
            << QStringLiteral("audio/aiff")
            << QStringLiteral("audio/amr")
            << QStringLiteral("audio/amr-wb")
            << QStringLiteral("audio/basic")
            << QStringLiteral("audio/x-ape")
            << QStringLiteral("audio/x-wavpack")
            << QStringLiteral("audio/x-shorten")
            << QStringLiteral("video/vnd.dlna.mpeg-tts")
            << QStringLiteral("audio/vnd.dlna.adts")
            << QStringLiteral("audio/mpeg")
            << QStringLiteral("video/mpeg")
            << QStringLiteral("video/dvd")
            << QStringLiteral("video/mp4")
            << QStringLiteral("audio/mp4")
            << QStringLiteral("audio/aac")
            << QStringLiteral("audio/flac")
            << QStringLiteral("audio/ogg")
            << QStringLiteral("video/ogg")
            << QStringLiteral("application/ogg")
            << QStringLiteral("video/x-matroska")
            << QStringLiteral("audio/x-matroska")
            << QStringLiteral("video/webm")
            << QStringLiteral("audio/webm")
            << QStringLiteral("video/avi")
            << QStringLiteral("video/x-msvideo")
            << QStringLiteral("video/flc")
            << QStringLiteral("application/gxf")
            << QStringLiteral("application/mxf")
            << QStringLiteral("audio/x-ms-wma")
            << QStringLiteral("video/x-ms-wm")
            << QStringLiteral("video/x-ms-wmv")
            << QStringLiteral("video/x-ms-asf")
            << QStringLiteral("video/x-flv")
            << QStringLiteral("video/mp4")
            << QStringLiteral("audio/mp4")
            << QStringLiteral("video/quicktime")
            << QStringLiteral("application/vnd.rn-realmedia")
            << QStringLiteral("application/vnd.rn-realmedia-vbr")
            << QStringLiteral("audio/vnd.rn-realaudio")
            << QStringLiteral("audio/3gpp")
            << QStringLiteral("audio/3gpp2")
            << QStringLiteral("video/3gpp")
            << QStringLiteral("video/3gpp2")
            << QStringLiteral("audio/x-mpegurl")
            << QStringLiteral("audio/x-scpls"));
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
            mSuffix.prepend(QLatin1String("*."));
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

/*QStringList externalFilesToLoad(const QFile &originalMediaFile, const QString &fileType)
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
        const QFileInfo& fi(fileList.at(i));
        if (fi.absoluteFilePath() == originalMediaFile.absoluteFilePath())
        {
            continue;
        }
        const QString newBaseName = fi.baseName().toLower();
        if (newBaseName == fileBaseName)
        {
            if (fileType.toLower() == QLatin1String("sub"))
            {
                if (fi.suffix().toLower() == QLatin1String("ass")
                        || fi.suffix().toLower() == QLatin1String("ssa")
                        || fi.suffix().toLower() == QLatin1String("srt")
                        || fi.suffix().toLower() == QLatin1String("sup"))
                {
                    newFileList.append(QDir::toNativeSeparators(fi.absoluteFilePath()));
                }
            }
            else if (fileType.toLower() == QLatin1String("audio"))
            {
                if (fi.suffix().toLower() == QLatin1String("mka"))
                {
                    newFileList.append(QDir::toNativeSeparators(fi.absoluteFilePath()));
                }
            }
        }
    }
    return newFileList;
}*/

QString LogFileLocation()
{
    QString logPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + QDir::separator();
#ifdef Q_OS_WIN64
    logPath += QLatin1String("debug64.log");
#else
    logPath += QLatin1String("debug.log");
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
        msgType = QStringLiteral("DEBUG");
        break;
    case QtInfoMsg:
        msgType = QStringLiteral("INFORMATION");
        break;
    case QtWarningMsg:
        msgType = QStringLiteral("WARNING");
        break;
    case QtCriticalMsg:
        msgType = QStringLiteral("CRITICAL");
        break;
    case QtFatalMsg:
        msgType = QStringLiteral("FATAL");
        break;
    /*case QtSystemMsg:
        msgType = QStringLiteral("SYSTEM");
        break;*/
    default:
        msgType = QStringLiteral("DEBUG");
        break;
    }

    QString dateTimeStr = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss ddd"));
    QString messageStr = QStringLiteral("[%1] | Message: %2 | File: %3 | Line: %4 | Function: %5 | DateTime: %6 ;")
                .arg(msgType).arg(msg).arg(context.file).arg(context.line).arg(context.function).arg(dateTimeStr);

    QFile file(LogFileLocation());
    file.open(QFile::WriteOnly | QFile::Append | QFile::Text);
    QTextStream ts(&file);
    ts << messageStr << "\n";
    file.flush();
    file.close();

    mutex.unlock();

#ifdef _DEBUG
    fprintf_s(stderr, "%s\n", messageStr.toLocal8Bit().constData());
#endif
}

QString SnapDirLocation()
{
    QString snapDirPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    return QDir::toNativeSeparators(snapDirPath);
}

QString HumanSize(qint64 size)
{
    // taken from http://comments.gmane.org/gmane.comp.lib.qt.general/34914
    float num = size;
    QStringList list;
    list << QStringLiteral("KB") << QStringLiteral("MB") << QStringLiteral("GB") << QStringLiteral("TB");

    QStringListIterator i(list);
    QString unit(QStringLiteral("bytes"));

    while(num >= 1024.0 && i.hasNext())
     {
        unit = i.next();
        num /= 1024.0;
    }
    return QString().setNum(num,'f',2)+" "+unit;
}

QString FormatTime(int _time, int _totalTime)
{
    QTime time = QTime::fromMSecsSinceStartOfDay(_time * 1000);
    if(_totalTime >= 3600) // hours
        return time.toString(QStringLiteral("h:mm:ss"));
    if(_totalTime >= 60)   // minutes
        return time.toString(QStringLiteral("mm:ss"));
    return time.toString(QStringLiteral("0:ss"));   // seconds
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
        return QStringLiteral("0:0");
    return QStringLiteral("%0:%1").arg(QString::number(w/gcd), QString::number(h/gcd));
}

bool IsValidUrl(const QString& url)
{
    QRegExp rx("^[a-z]{2,}://", Qt::CaseInsensitive); // url
    return (rx.indexIn(url) != -1);
}

bool IsValidFile(const QString& path)
{
    QRegExp rx(R"(^(\.{1,2}|[a-z]:|\\\\))", Qt::CaseInsensitive); // relative path, network location, drive
    return (rx.indexIn(path) != -1);
}

bool IsValidLocation(const QString& loc)
{
    QRegExp rx(R"(^([a-z]{2,}://|\.{1,2}|[a-z]:|\\\\))", Qt::CaseInsensitive); // url, relative path, network location, drive
    return (rx.indexIn(loc) != -1);
}

QString MonospaceFont()
{
    return QStringLiteral("Lucida Console");
}

QString FormatNumberWithAmpersand(int val, int length)
{
    if(length < 10)
        return "&"+QString::number(val);
    if(length < 100)
    {
        if(val < 10)
            return "0&"+QString::number(val);
        return QStringLiteral("%1").arg(val, 2, 10, QChar('0'));
    }
    if(val < 10)
        return "00&"+QString::number(val);
    return QStringLiteral("%1").arg(val, 3, 10, QChar('0'));
}

QString ShortenPathToParent(const Recent &recent)
{
    const int long_name = 100;
    if(recent.title != QString())
        return QStringLiteral("%0 (%1)").arg(recent.title, recent.path);
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
                parent += QLatin1String("..");
            }
            if(file.length() > long_name)
            {
                file.truncate(long_name);
                i = p.lastIndexOf('.');
                file += QLatin1String("..");
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

}
