#ifndef SKINMANAGER_H
#define SKINMANAGER_H

#include <QObject>

class SkinManager : public QObject
{
    Q_OBJECT
public:
    static SkinManager *instance();

signals:

public slots:
    QString currentSkinName() const;
    QString currentSkinPath() const;
    QString currentSkinContent() const;
    bool setSkin(const QString &skin = QString::fromLatin1("Default"));

private:
    explicit SkinManager(QObject *parent = nullptr);

private:
    QString curSkinName;
    QString curSkinPath;
    QString curSkinContent;
};

#endif // SKINMANAGER_H
