#pragma once

#include <QObject>

#ifdef Q_OS_WIN

class FileAssoc : public QObject
{
    Q_OBJECT

public:
    enum reg_state
    {
        NOT_REGISTERED,
        SOME_REGISTERED,
        ALL_REGISTERED
    };

    enum reg_type
    {
        NONE,
        VIDEO_ONLY,
        AUDIO_ONLY,
        ALL
    };

    explicit FileAssoc(QObject *parent = nullptr);

private:
    reg_type regType = reg_type::NONE;

signals:

public slots:
    bool registerMediaFiles(reg_type type = reg_type::ALL);
    void unregisterMediaFiles(reg_type type = reg_type::ALL);
    reg_state getMediaFilesRegisterState();

private slots:
    void deleteRegistryKey(const QString &key);
    bool add_verbs(const QString &key);
    bool add_progid(const QString &prog_id, const QString &friendly_name, const QString &icon_path);
    bool update_extension(const QString &extension, const QString &prog_id, const QString &mime_type, const QString &perceived_type);
    bool add_type(const QString &mime_type, const QString &perceived_type, const QString &friendly_name, const QString &extension);
};

#endif
