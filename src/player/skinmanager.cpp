#include "skinmanager.h"

#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDir>

SkinManager::SkinManager(QObject *parent) : QObject(parent)
{

}

SkinManager *SkinManager::instance()
{
    static SkinManager skinManager;
    return &skinManager;
}

QString SkinManager::currentSkinName() const
{
    return curSkinName;
}

QString SkinManager::currentSkinPath() const
{
    return curSkinPath;
}

QString SkinManager::currentSkinContent() const
{
    return curSkinContent;
}

bool SkinManager::setSkin(const QString &skin)
{
    if (skin.isEmpty())
    {
        return false;
    }
    QString filePath;
    if (skin.contains(QString::fromLatin1("/")) || skin.contains(QString::fromLatin1("\\")))
    {
        filePath = skin;
    }
    else
    {
        filePath = QApplication::applicationDirPath() + QDir::separator() + QString::fromLatin1("stylesheets")
                + QDir::separator() + skin + QString::fromLatin1(".css");
    }
    if (filePath.isEmpty())
    {
        return false;
    }
    if (!QFileInfo(filePath).exists())
    {
        return false;
    }
    if (!QFileInfo(filePath).isFile())
    {
        return false;
    }
    QFile skinFile(filePath);
    if (skinFile.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream ts(&skinFile);
        curSkinContent = ts.readAll();
        skinFile.close();
        if (curSkinContent.isEmpty())
        {
            return false;
        }
        QApplication *app = qApp;
        app->setStyleSheet(curSkinContent);
        return true;
    }
    return false;
}
