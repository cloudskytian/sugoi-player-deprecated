#ifndef UTIL_H
#define UTIL_H

#include <QTextStream>

#include "recent.h"

//class QWidget;
//class QFile;
class QFileInfo;

namespace Util {

QStringList supportedMimeTypes();
QStringList supportedSuffixes();
bool executeProgramWithAdministratorPrivilege(const QString &exePath, const QString &exeParam);

//QStringList externalFilesToLoad(const QFile &originalMediaFile, const QString &fileType);
//QStringList externalFilesToLoad(const QFileInfo &originalMediaFile, const QString &fileType);

//void SetAlwaysOnTop(QWidget *window, bool);
QString SettingsLocation();
QString LogFileLocation();
QString SnapDirLocation();

void messagesOutputToFile(QtMsgType type, const QMessageLogContext &context, const QString &msg);

void ShowInFolder(const QString& path, const QString& file);
}

#endif // UTIL_H
