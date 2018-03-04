#ifndef UTIL_H
#define UTIL_H

#include <QWidget>
#include <QString>
#include <QTextStream>
#include <QFile>
#include <QFileInfo>

#include "recent.h"

namespace Util {

QStringList supportedMimeTypes();
QStringList supportedSuffixes();
bool executeProgramWithAdministratorPrivilege(const QString &exePath, const QString &exeParam);
bool setAutoStart(const QString &path, const QString &param);
bool isAutoStart();
bool disableAutoStart();

QStringList externalFilesToLoad(const QFile &originalMediaFile, const QString &fileType);
QStringList externalFilesToLoad(const QFileInfo &originalMediaFile, const QString &fileType);

bool DimLightsSupported();
void SetAlwaysOnTop(QWidget *window, bool);
QString SettingsLocation();
QString LogFileLocation();

void messagesOutputToFile(QtMsgType type, const QMessageLogContext &context, const QString &msg);

bool IsValidFile(QString path);
bool IsValidLocation(QString loc); // combined file and url

void ShowInFolder(QString path, QString file);

QString MonospaceFont();

// common
bool IsValidUrl(QString url);

QString FormatTime(int time, int totalTime);
QString FormatRelativeTime(int time);
QString FormatNumber(int val, int length);
QString FormatNumberWithAmpersand(int val, int length);
QString HumanSize(qint64);
QString ShortenPathToParent(const Recent &recent);
QStringList ToNativeSeparators(QStringList list);
QStringList FromNativeSeparators(QStringList list);
int GCD(int v, int u);
QString Ratio(int w, int h);

}

inline QTextStream& qStdout()
{
    static QTextStream r{stdout};
    return r;
}

#endif // UTIL_H
