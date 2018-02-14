#include "fileassoc.h"

#include <QSettings>
#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>
#include <QLibrary>

#include <Windows.h>
#include <Shlobj.h>

FileAssoc::FileAssoc(QObject *parent) : QObject(parent)
{

}

FileAssoc::reg_state FileAssoc::getMediaFilesRegisterState()
{
    const QString videoKey = QString::fromLatin1("HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\io.SPlayer.avi");
    const QString audioKey = QString::fromLatin1("HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\io.SPlayer.mp3");
    QSettings videoSettings(videoKey, QSettings::NativeFormat);
    QSettings audioSettings(audioKey, QSettings::NativeFormat);
    if (videoSettings.contains(QString::fromLatin1("FriendlyTypeName"))
            && audioSettings.contains(QString::fromLatin1("FriendlyTypeName")))
    {
        return FileAssoc::reg_state::ALL_REGISTERED;
    }
    else if (videoSettings.contains(QString::fromLatin1("FriendlyTypeName"))
             || audioSettings.contains(QString::fromLatin1("FriendlyTypeName")))
    {
        return FileAssoc::reg_state::SOME_REGISTERED;
    }
    return FileAssoc::reg_state::NOT_REGISTERED;
}

void FileAssoc::deleteRegistryKey(const QString &key)
{
    QSettings settings(key, QSettings::NativeFormat);
    settings.clear();
}

void FileAssoc::unregisterMediaFiles(reg_type type)
{
    if (type == FileAssoc::reg_type::NONE)
    {
        return;
    }

    const QString filePath = QCoreApplication::applicationFilePath();
    const QString fileName = QFileInfo(filePath).fileName();

    // Delete "App Paths" entry
    const QString key = QString::fromLatin1("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\") + fileName;
    deleteRegistryKey(key);

    // Delete HKLM subkeys
    const QString key2 = QString::fromLatin1("HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\Applications\\") + fileName;
    deleteRegistryKey(key2);

    const QString key3 = QString::fromLatin1("HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\SystemFileAssociations\\video\\OpenWithList\\") + fileName;
    deleteRegistryKey(key3);

    const QString key4 = QString::fromLatin1("HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\SystemFileAssociations\\audio\\OpenWithList\\") + fileName;
    deleteRegistryKey(key4);

    // Delete "Default Programs" entry
    const QString key5 = QString::fromLatin1("HKEY_LOCAL_MACHINE\\SOFTWARE\\RegisteredApplications");
    QSettings settings5(key5, QSettings::NativeFormat);
    settings5.remove(QString::fromLatin1("SPlayer"));

    const QString key6 = QString::fromLatin1("HKEY_LOCAL_MACHINE\\SOFTWARE\\Clients\\Media\\SPlayer\\Capabilities");
    deleteRegistryKey(key6);

    // Delete all OpenWithProgIds referencing ProgIds that start with io.SPlayer.
    // TODO

    // Delete all ProgIds starting with io.SPlayer.
    const QString key8 = QString::fromLatin1("HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes");
    QSettings settings8(key8, QSettings::NativeFormat);
    QStringList key8s = settings8.allKeys();
    for (int i = 0; i <= (key8s.count() - 1); ++i)
    {
        QString currentKeyName = key8s.at(i);
        if (currentKeyName.startsWith(QString::fromLatin1("io.SPlayer")))
        {
            settings8.remove(currentKeyName);
        }
    }

    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

    if (type == FileAssoc::reg_type::VIDEO_ONLY)
    {
        registerMediaFiles(FileAssoc::reg_type::AUDIO_ONLY);
    }
    else if (type == FileAssoc::reg_type::AUDIO_ONLY)
    {
        registerMediaFiles(FileAssoc::reg_type::VIDEO_ONLY);
    }
}

bool FileAssoc::add_verbs(const QString &key)
{
    QSettings settings(key, QSettings::NativeFormat);
    if (settings.status() != QSettings::NoError)
    {
        return false;
    }

    const QString filePath = QCoreApplication::applicationFilePath();

    settings.setValue(QString::fromLatin1("FriendlyAppName"), QString::fromLatin1("SPlayer"));
    settings.beginGroup(QString::fromLatin1("shell"));
    // Set the default verb to "play"
    settings.setValue(QString::fromLatin1("."), QString::fromLatin1("play"));
    // Hide the "open" verb from the context menu, since it's the same as "play"
    settings.beginGroup(QString::fromLatin1("open"));
    settings.setValue(QString::fromLatin1("LegacyDisable"), QString());
    settings.beginGroup(QString::fromLatin1("command"));
    // Set open command
    settings.setValue(QString::fromLatin1(".")
                      , QLatin1Char('"') + QDir::toNativeSeparators(filePath)
                      + QString::fromLatin1("\" \"%1\""));
    settings.endGroup();
    settings.endGroup();
    // Add "play" verb
    settings.beginGroup(QString::fromLatin1("play"));
    settings.setValue(QString::fromLatin1("."), QString::fromLatin1("&Play"));
    settings.beginGroup(QString::fromLatin1("command"));
    settings.setValue(QString::fromLatin1(".")
                      , QLatin1Char('"') + QDir::toNativeSeparators(filePath)
                      + QString::fromLatin1("\" \"%1\""));
    settings.endGroup();
    settings.endGroup();
    settings.endGroup();

    return true;
}

bool FileAssoc::add_progid(const QString &prog_id, const QString &friendly_name, const QString &icon_path)
{
    // Add ProgId, edit flags are FTA_OpenIsSafe | FTA_AlwaysUseDirectInvoke
    const QString key = QString::fromLatin1("HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\") + prog_id;
    QSettings settings(key, QSettings::NativeFormat);
    if (settings.status() != QSettings::NoError)
    {
        return false;
    }

    settings.setValue(QString::fromLatin1("."), friendly_name);
    settings.setValue(QString::fromLatin1("EditFlags"), 4259840);
    settings.setValue(QString::fromLatin1("FriendlyTypeName"), friendly_name);
    settings.beginGroup(QString::fromLatin1("DefaultIcon"));
    settings.setValue(QString::fromLatin1("."), QDir::toNativeSeparators(icon_path));
    settings.endGroup();

    return add_verbs(key);
}

bool FileAssoc::update_extension(const QString &extension, const QString &prog_id, const QString &mime_type, const QString &perceived_type)
{
    // Add information about the file extension
    const QString key = QString::fromLatin1("HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\") + extension;
    QSettings settings(key, QSettings::NativeFormat);
    if (settings.status() != QSettings::NoError)
    {
        return false;
    }

    settings.setValue(QString::fromLatin1("Content Type"), mime_type);
    settings.setValue(QString::fromLatin1("PerceivedType"), perceived_type);
    settings.beginGroup(QString::fromLatin1("OpenWithProgIds"));
    settings.setValue(prog_id, QString());
    settings.endGroup();

    // Add type to SupportedTypes
    const QString filePath = QCoreApplication::applicationFilePath();
    const QString fileName = QFileInfo(filePath).fileName();
    const QString key2 = QString::fromLatin1("HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\Applications\\") + fileName + ("\\SupportedTypes");
    QSettings settings2(key2, QSettings::NativeFormat);
    if (settings2.status() != QSettings::NoError)
    {
        return false;
    }

    settings2.setValue(extension, QString());

    // Add type to the Default Programs control panel
    const QString key3 = QString::fromLatin1("HKEY_LOCAL_MACHINE\\SOFTWARE\\Clients\\Media\\SPlayer\\Capabilities\\FileAssociations");
    QSettings settings3(key3, QSettings::NativeFormat);
    if (settings3.status() != QSettings::NoError)
    {
        return false;
    }

    settings3.setValue(extension, prog_id);

    return true;
}

bool FileAssoc::add_type(const QString &mime_type, const QString &perceived_type, const QString &friendly_name, const QString &extension)
{
    if (regType == FileAssoc::reg_type::NONE)
    {
        return false;
    }
    else if (regType == FileAssoc::reg_type::VIDEO_ONLY)
    {
        if (perceived_type == QString::fromLatin1("audio"))
        {
            return false;
        }
    }
    else if (regType == FileAssoc::reg_type::AUDIO_ONLY)
    {
        if (perceived_type == QString::fromLatin1("video"))
        {
            return false;
        }
    }
    // Add ProgId
    const QString prog_id = QString::fromLatin1("io.SPlayer") + extension;
    QString iconPath = QLatin1Char('"') + QCoreApplication::applicationFilePath() + QString::fromLatin1("\",0");
    const QString iconLibPath = QCoreApplication::applicationDirPath() + QLatin1Char('/') + QString::fromLatin1("iconlib.dll");
    QLibrary iconLib;
    iconLib.setFileName(iconLibPath);
    if (iconLib.load())
    {
        typedef int (*GetIconIndex)(LPCTSTR);
        GetIconIndex iconIndex = (GetIconIndex)iconLib.resolve("GetIconIndex");
        if (iconIndex)
        {
            int index = iconIndex(reinterpret_cast<const wchar_t *>(extension.utf16()));
            index = index > -1 ? index : 0;
            QString str = QString::number(index);
            str = str.remove("\"");
            iconPath = QLatin1Char('"') + iconLibPath + QString::fromLatin1("\",") + str;
        }
    }
    if (!add_progid(prog_id, friendly_name, iconPath))
    {
        return false;
    }

    // Add extensions
    if (!update_extension(extension, prog_id, mime_type, perceived_type))
    {
        return false;
    }

    return true;
}

bool FileAssoc::registerMediaFiles(FileAssoc::reg_type type)
{
    regType = type;

    if (regType == FileAssoc::reg_type::NONE)
    {
        unregisterMediaFiles(FileAssoc::reg_type::ALL);
        return true;
    }

    const QString filePath = QCoreApplication::applicationFilePath();
    const QString fileName = QFileInfo(filePath).fileName();

    // Register splayer.exe under the "App Paths" key, so it can be found by
    // ShellExecute, the run command, the start menu, etc.
    const QString key1 = QString::fromLatin1("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\") + fileName;
    QSettings settings1(key1, QSettings::NativeFormat);
    if (settings1.status() != QSettings::NoError)
    {
        return false;
    }

    settings1.setValue(QString::fromLatin1("."), QDir::toNativeSeparators(filePath));
    settings1.setValue(QString::fromLatin1("UseUrl"), 1);

    // Register splayer.exe under the "Applications" key to add some default verbs for
    // when SPlayer is used from the "Open with" menu
    const QString key2 = QString::fromLatin1("HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\Applications\\") + fileName;
    if (!add_verbs(key2))
    {
        return false;
    }

    // Add SPlayer to the "Open with" list for all video and audio file types
    if (regType != FileAssoc::reg_type::AUDIO_ONLY)
    {
        const QString key3 = QString::fromLatin1("HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\SystemFileAssociations\\video\\OpenWithList\\") + fileName;
        QSettings settings3(key3, QSettings::NativeFormat);
        if (settings3.status() != QSettings::NoError)
        {
            return false;
        }

        settings3.setValue(QString::fromLatin1("FileName"), fileName);
    }

    if (regType != FileAssoc::reg_type::VIDEO_ONLY)
    {
        const QString key4 = QString::fromLatin1("HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\SystemFileAssociations\\audio\\OpenWithList\\") + fileName;
        QSettings settings4(key4, QSettings::NativeFormat);
        if (settings4.status() != QSettings::NoError)
        {
            return false;
        }

        settings4.setValue(QString::fromLatin1("FileName"), fileName);
    }

    // Add a capabilities key for SPlayer, which is registered later on for use in the
    // "Default Programs" control panel
    const QString key5 = QString::fromLatin1("HKEY_LOCAL_MACHINE\\SOFTWARE\\Clients\\Media\\SPlayer\\Capabilities");
    QSettings settings5(key5, QSettings::NativeFormat);
    if (settings5.status() != QSettings::NoError)
    {
        return false;
    }

    settings5.setValue(QString::fromLatin1("ApplicationName"), QString::fromLatin1("SPlayer"));
    settings5.setValue(QString::fromLatin1("ApplicationDescription"), QString::fromLatin1("SPlayer, a multimedia player for Windows 7+ based on libmpv and Qt."));

    // Add file types
    // DVD/Blu-ray audio formats
    add_type(QString::fromLatin1("audio/ac3"), QString::fromLatin1("audio"), QString::fromLatin1("AC-3 Audio"), QString::fromLatin1(".ac3"));
    add_type(QString::fromLatin1("audio/ac3"), QString::fromLatin1("audio"), QString::fromLatin1("AC-3 Audio"), QString::fromLatin1(".a52"));
    add_type(QString::fromLatin1("audio/eac3"), QString::fromLatin1("audio"), QString::fromLatin1("E-AC-3 Audio"), QString::fromLatin1(".eac3"));
    add_type(QString::fromLatin1("audio/vnd.dolby.mlp"), QString::fromLatin1("audio"), QString::fromLatin1("MLP Audio"), QString::fromLatin1(".mlp"));
    add_type(QString::fromLatin1("audio/vnd.dts"), QString::fromLatin1("audio"), QString::fromLatin1("DTS Audio"), QString::fromLatin1(".dts"));
    add_type(QString::fromLatin1("audio/vnd.dts.hd"), QString::fromLatin1("audio"), QString::fromLatin1("DTS-HD Audio"), QString::fromLatin1(".dts-hd"));
    add_type(QString::fromLatin1("audio/vnd.dts.hd"), QString::fromLatin1("audio"), QString::fromLatin1("DTS-HD Audio"), QString::fromLatin1(".dtshd"));
    add_type(QString(), QString::fromLatin1("audio"), QString::fromLatin1("TrueHD Audio"), QString::fromLatin1(".true-hd"));
    add_type(QString(), QString::fromLatin1("audio"), QString::fromLatin1("TrueHD Audio"), QString::fromLatin1(".thd"));
    add_type(QString(), QString::fromLatin1("audio"), QString::fromLatin1("TrueHD Audio"), QString::fromLatin1(".truehd"));
    add_type(QString(), QString::fromLatin1("audio"), QString::fromLatin1("TrueHD Audio"), QString::fromLatin1(".thd+ac3"));
    add_type(QString(), QString::fromLatin1("audio"), QString::fromLatin1("True Audio"), QString::fromLatin1(".tta"));
    // Uncompressed formats
    add_type(QString(), QString::fromLatin1("audio"), QString::fromLatin1("PCM Audio"), QString::fromLatin1(".pcm"));
    add_type(QString::fromLatin1("audio/wav"), QString::fromLatin1("audio"), QString::fromLatin1("Wave Audio"), QString::fromLatin1(".wav"));
    add_type(QString::fromLatin1("audio/aiff"), QString::fromLatin1("audio"), QString::fromLatin1("AIFF Audio"), QString::fromLatin1(".aiff"));
    add_type(QString::fromLatin1("audio/aiff"), QString::fromLatin1("audio"), QString::fromLatin1("AIFF Audio"), QString::fromLatin1(".aif"));
    add_type(QString::fromLatin1("audio/aiff"), QString::fromLatin1("audio"), QString::fromLatin1("AIFF Audio"), QString::fromLatin1(".aifc"));
    add_type(QString::fromLatin1("audio/amr"), QString::fromLatin1("audio"), QString::fromLatin1("AMR Audio"), QString::fromLatin1(".amr"));
    add_type(QString::fromLatin1("audio/amr-wb"), QString::fromLatin1("audio"), QString::fromLatin1("AMR-WB Audio"), QString::fromLatin1(".awb"));
    add_type(QString::fromLatin1("audio/basic"), QString::fromLatin1("audio"), QString::fromLatin1("AU Audio"), QString::fromLatin1(".au"));
    add_type(QString::fromLatin1("audio/basic"), QString::fromLatin1("audio"), QString::fromLatin1("AU Audio"), QString::fromLatin1(".snd"));
    add_type(QString(), QString::fromLatin1("audio"), QString::fromLatin1("Linear PCM Audio"), QString::fromLatin1(".lpcm"));
    add_type(QString(), QString::fromLatin1("video"), QString::fromLatin1("Raw YUV Video"), QString::fromLatin1(".yuv"));
    add_type(QString(), QString::fromLatin1("video"), QString::fromLatin1("YUV4MPEG2 Video"), QString::fromLatin1(".y4m"));
    // Free lossless formats
    add_type(QString::fromLatin1("audio/x-ape"), QString::fromLatin1("audio"), QString::fromLatin1("Monkey's Audio"), QString::fromLatin1(".ape"));
    add_type(QString::fromLatin1("audio/x-wavpack"), QString::fromLatin1("audio"), QString::fromLatin1("WavPack Audio"), QString::fromLatin1(".wv"));
    add_type(QString::fromLatin1("audio/x-shorten"), QString::fromLatin1("audio"), QString::fromLatin1("Shorten Audio"), QString::fromLatin1(".shn"));
    // MPEG formats
    add_type(QString::fromLatin1("video/vnd.dlna.mpeg-tts"), QString::fromLatin1("video"), QString::fromLatin1("MPEG-2 Transport Stream"), QString::fromLatin1(".m2ts"));
    add_type(QString::fromLatin1("video/vnd.dlna.mpeg-tts"), QString::fromLatin1("video"), QString::fromLatin1("MPEG-2 Transport Stream"), QString::fromLatin1(".m2t"));
    add_type(QString::fromLatin1("video/vnd.dlna.mpeg-tts"), QString::fromLatin1("video"), QString::fromLatin1("MPEG-2 Transport Stream"), QString::fromLatin1(".mts"));
    add_type(QString::fromLatin1("video/vnd.dlna.mpeg-tts"), QString::fromLatin1("video"), QString::fromLatin1("MPEG-2 Transport Stream"), QString::fromLatin1(".mtv"));
    add_type(QString::fromLatin1("video/vnd.dlna.mpeg-tts"), QString::fromLatin1("video"), QString::fromLatin1("MPEG-2 Transport Stream"), QString::fromLatin1(".ts"));
    add_type(QString::fromLatin1("video/vnd.dlna.mpeg-tts"), QString::fromLatin1("video"), QString::fromLatin1("MPEG-2 Transport Stream"), QString::fromLatin1(".tsv"));
    add_type(QString::fromLatin1("video/vnd.dlna.mpeg-tts"), QString::fromLatin1("video"), QString::fromLatin1("MPEG-2 Transport Stream"), QString::fromLatin1(".tsa"));
    add_type(QString::fromLatin1("video/vnd.dlna.mpeg-tts"), QString::fromLatin1("video"), QString::fromLatin1("MPEG-2 Transport Stream"), QString::fromLatin1(".tts"));
    add_type(QString::fromLatin1("video/vnd.dlna.mpeg-tts"), QString::fromLatin1("video"), QString::fromLatin1("MPEG-2 Transport Stream"), QString::fromLatin1(".trp"));
    add_type(QString::fromLatin1("audio/vnd.dlna.adts"), QString::fromLatin1("audio"), QString::fromLatin1("ADTS Audio"), QString::fromLatin1(".adts"));
    add_type(QString::fromLatin1("audio/vnd.dlna.adts"), QString::fromLatin1("audio"), QString::fromLatin1("ADTS Audio"), QString::fromLatin1(".adt"));
    add_type(QString::fromLatin1("audio/mpeg"), QString::fromLatin1("audio"), QString::fromLatin1("MPEG Audio"), QString::fromLatin1(".mpa"));
    add_type(QString::fromLatin1("audio/mpeg"), QString::fromLatin1("audio"), QString::fromLatin1("MPEG Audio"), QString::fromLatin1(".m1a"));
    add_type(QString::fromLatin1("audio/mpeg"), QString::fromLatin1("audio"), QString::fromLatin1("MPEG Audio"), QString::fromLatin1(".m2a"));
    add_type(QString::fromLatin1("audio/mpeg"), QString::fromLatin1("audio"), QString::fromLatin1("MPEG Audio"), QString::fromLatin1(".mp1"));
    add_type(QString::fromLatin1("audio/mpeg"), QString::fromLatin1("audio"), QString::fromLatin1("MPEG Audio"), QString::fromLatin1(".mp2"));
    add_type(QString::fromLatin1("audio/mpeg"), QString::fromLatin1("audio"), QString::fromLatin1("MP3 Audio"), QString::fromLatin1(".mp3"));
    add_type(QString::fromLatin1("video/mpeg"), QString::fromLatin1("video"), QString::fromLatin1("MPEG Video"), QString::fromLatin1(".mpeg"));
    add_type(QString::fromLatin1("video/mpeg"), QString::fromLatin1("video"), QString::fromLatin1("MPEG Video"), QString::fromLatin1(".mpg"));
    add_type(QString::fromLatin1("video/mpeg"), QString::fromLatin1("video"), QString::fromLatin1("MPEG Video"), QString::fromLatin1(".mpe"));
    add_type(QString::fromLatin1("video/mpeg"), QString::fromLatin1("video"), QString::fromLatin1("MPEG Video"), QString::fromLatin1(".mpeg2"));
    add_type(QString::fromLatin1("video/mpeg"), QString::fromLatin1("video"), QString::fromLatin1("MPEG Video"), QString::fromLatin1(".m1v"));
    add_type(QString::fromLatin1("video/mpeg"), QString::fromLatin1("video"), QString::fromLatin1("MPEG Video"), QString::fromLatin1(".m2v"));
    add_type(QString::fromLatin1("video/mpeg"), QString::fromLatin1("video"), QString::fromLatin1("MPEG Video"), QString::fromLatin1(".mp2v"));
    add_type(QString::fromLatin1("video/mpeg"), QString::fromLatin1("video"), QString::fromLatin1("MPEG Video"), QString::fromLatin1(".mpv"));
    add_type(QString::fromLatin1("video/mpeg"), QString::fromLatin1("video"), QString::fromLatin1("MPEG Video"), QString::fromLatin1(".mpv2"));
    add_type(QString::fromLatin1("video/mpeg"), QString::fromLatin1("video"), QString::fromLatin1("MPEG Video"), QString::fromLatin1(".mod"));
    add_type(QString::fromLatin1("video/mpeg"), QString::fromLatin1("video"), QString::fromLatin1("MPEG Video"), QString::fromLatin1(".tod"));
    add_type(QString::fromLatin1("video/dvd"), QString::fromLatin1("video"), QString::fromLatin1("Video Object"), QString::fromLatin1(".vob"));
    add_type(QString::fromLatin1("video/dvd"), QString::fromLatin1("video"), QString::fromLatin1("Video Object"), QString::fromLatin1(".vro"));
    add_type(QString(), QString::fromLatin1("video"), QString::fromLatin1("Enhanced VOB"), QString::fromLatin1(".evob"));
    add_type(QString(), QString::fromLatin1("video"), QString::fromLatin1("Enhanced VOB"), QString::fromLatin1(".evo"));
    add_type(QString::fromLatin1("video/mp4"), QString::fromLatin1("video"), QString::fromLatin1("MPEG-4 Video"), QString::fromLatin1(".mpeg4"));
    add_type(QString::fromLatin1("video/mp4"), QString::fromLatin1("video"), QString::fromLatin1("MPEG-4 Video"), QString::fromLatin1(".m4v"));
    add_type(QString::fromLatin1("video/mp4"), QString::fromLatin1("video"), QString::fromLatin1("MPEG-4 Video"), QString::fromLatin1(".mp4"));
    add_type(QString::fromLatin1("video/mp4"), QString::fromLatin1("video"), QString::fromLatin1("MPEG-4 Video"), QString::fromLatin1(".mp4v"));
    add_type(QString::fromLatin1("video/mp4"), QString::fromLatin1("video"), QString::fromLatin1("MPEG-4 Video"), QString::fromLatin1(".mpg4"));
    add_type(QString::fromLatin1("audio/mp4"), QString::fromLatin1("audio"), QString::fromLatin1("MPEG-4 Audio"), QString::fromLatin1(".m4a"));
    add_type(QString::fromLatin1("audio/aac"), QString::fromLatin1("audio"), QString::fromLatin1("Raw AAC Audio"), QString::fromLatin1(".aac"));
    add_type(QString(), QString::fromLatin1("video"), QString::fromLatin1("Raw H.264/AVC Video"), QString::fromLatin1(".h264"));
    add_type(QString(), QString::fromLatin1("video"), QString::fromLatin1("Raw H.264/AVC Video"), QString::fromLatin1(".avc"));
    add_type(QString(), QString::fromLatin1("video"), QString::fromLatin1("Raw H.264/AVC Video"), QString::fromLatin1(".x264"));
    add_type(QString(), QString::fromLatin1("video"), QString::fromLatin1("Raw H.264/AVC Video"), QString::fromLatin1(".264"));
    add_type(QString(), QString::fromLatin1("video"), QString::fromLatin1("Raw H.265/HEVC Video"), QString::fromLatin1(".hevc"));
    add_type(QString(), QString::fromLatin1("video"), QString::fromLatin1("Raw H.265/HEVC Video"), QString::fromLatin1(".h265"));
    add_type(QString(), QString::fromLatin1("video"), QString::fromLatin1("Raw H.265/HEVC Video"), QString::fromLatin1(".x265"));
    add_type(QString(), QString::fromLatin1("video"), QString::fromLatin1("Raw H.265/HEVC Video"), QString::fromLatin1(".265"));
    // Xiph formats
    add_type(QString::fromLatin1("audio/flac"), QString::fromLatin1("audio"), QString::fromLatin1("FLAC Audio"), QString::fromLatin1(".flac"));
    add_type(QString::fromLatin1("audio/ogg"), QString::fromLatin1("audio"), QString::fromLatin1("Ogg Audio"), QString::fromLatin1(".oga"));
    add_type(QString::fromLatin1("audio/ogg"), QString::fromLatin1("audio"), QString::fromLatin1("Ogg Audio"), QString::fromLatin1(".ogg"));
    add_type(QString::fromLatin1("audio/ogg"), QString::fromLatin1("audio"), QString::fromLatin1("Opus Audio"), QString::fromLatin1(".opus"));
    add_type(QString::fromLatin1("audio/ogg"), QString::fromLatin1("audio"), QString::fromLatin1("Speex Audio"), QString::fromLatin1(".spx"));
    add_type(QString::fromLatin1("video/ogg"), QString::fromLatin1("video"), QString::fromLatin1("Ogg Video"), QString::fromLatin1(".ogv"));
    add_type(QString::fromLatin1("video/ogg"), QString::fromLatin1("video"), QString::fromLatin1("Ogg Video"), QString::fromLatin1(".ogm"));
    add_type(QString::fromLatin1("application/ogg"), QString::fromLatin1("video"), QString::fromLatin1("Ogg Video"), QString::fromLatin1(".ogx"));
    // Matroska formats
    add_type(QString::fromLatin1("video/x-matroska"), QString::fromLatin1("video"), QString::fromLatin1("Matroska Video"), QString::fromLatin1(".mkv"));
    add_type(QString::fromLatin1("video/x-matroska"), QString::fromLatin1("video"), QString::fromLatin1("Matroska 3D Video"), QString::fromLatin1(".mk3d"));
    add_type(QString::fromLatin1("audio/x-matroska"), QString::fromLatin1("audio"), QString::fromLatin1("Matroska Audio"), QString::fromLatin1(".mka"));
    add_type(QString::fromLatin1("video/webm"), QString::fromLatin1("video"), QString::fromLatin1("WebM Video"), QString::fromLatin1(".webm"));
    add_type(QString::fromLatin1("audio/webm"), QString::fromLatin1("audio"), QString::fromLatin1("WebM Audio"), QString::fromLatin1(".weba"));
    // Misc formats
    add_type(QString::fromLatin1("video/avi"), QString::fromLatin1("video"), QString::fromLatin1("Video Clip"), QString::fromLatin1(".avi"));
    add_type(QString::fromLatin1("video/avi"), QString::fromLatin1("video"), QString::fromLatin1("Video Clip"), QString::fromLatin1(".vfw"));
    add_type(QString(), QString::fromLatin1("video"), QString::fromLatin1("DivX Video"), QString::fromLatin1(".divx"));
    add_type(QString(), QString::fromLatin1("video"), QString::fromLatin1("3ivx Video"), QString::fromLatin1(".3iv"));
    add_type(QString(), QString::fromLatin1("video"), QString::fromLatin1("XVID Video"), QString::fromLatin1(".xvid"));
    add_type(QString(), QString::fromLatin1("video"), QString::fromLatin1("NUT Video"), QString::fromLatin1(".nut"));
    add_type(QString::fromLatin1("video/flc"), QString::fromLatin1("video"), QString::fromLatin1("FLIC Video"), QString::fromLatin1(".flic"));
    add_type(QString::fromLatin1("video/flc"), QString::fromLatin1("video"), QString::fromLatin1("FLIC Video"), QString::fromLatin1(".fli"));
    add_type(QString::fromLatin1("video/flc"), QString::fromLatin1("video"), QString::fromLatin1("FLIC Video"), QString::fromLatin1(".flc"));
    add_type(QString(), QString::fromLatin1("video"), QString::fromLatin1("Nullsoft Streaming Video"), QString::fromLatin1(".nsv"));
    add_type(QString::fromLatin1("application/gxf"), QString::fromLatin1("video"), QString::fromLatin1("General Exchange Format"), QString::fromLatin1(".gxf"));
    add_type(QString::fromLatin1("application/mxf"), QString::fromLatin1("video"), QString::fromLatin1("Material Exchange Format"), QString::fromLatin1(".mxf"));
    // Windows Media formats
    add_type(QString::fromLatin1("audio/x-ms-wma"), QString::fromLatin1("audio"), QString::fromLatin1("Windows Media Audio"), QString::fromLatin1(".wma"));
    add_type(QString::fromLatin1("video/x-ms-wm"), QString::fromLatin1("video"), QString::fromLatin1("Windows Media Video"), QString::fromLatin1(".wm"));
    add_type(QString::fromLatin1("video/x-ms-wmv"), QString::fromLatin1("video"), QString::fromLatin1("Windows Media Video"), QString::fromLatin1(".wmv"));
    add_type(QString::fromLatin1("video/x-ms-asf"), QString::fromLatin1("video"), QString::fromLatin1("Windows Media Video"), QString::fromLatin1(".asf"));
    add_type(QString(), QString::fromLatin1("video"), QString::fromLatin1("Microsoft Recorded TV Show"), QString::fromLatin1(".dvr-ms"));
    add_type(QString(), QString::fromLatin1("video"), QString::fromLatin1("Microsoft Recorded TV Show"), QString::fromLatin1(".dvr"));
    add_type(QString(), QString::fromLatin1("video"), QString::fromLatin1("Windows Recorded TV Show"), QString::fromLatin1(".wtv"));
    // DV formats
    add_type(QString(), QString::fromLatin1("video"), QString::fromLatin1("DV Video"), QString::fromLatin1(".dv"));
    add_type(QString(), QString::fromLatin1("video"), QString::fromLatin1("DV Video"), QString::fromLatin1(".hdv"));
    // Flash Video formats
    add_type(QString::fromLatin1("video/x-flv"), QString::fromLatin1("video"), QString::fromLatin1("Flash Video"), QString::fromLatin1(".flv"));
    add_type(QString::fromLatin1("video/mp4"), QString::fromLatin1("video"), QString::fromLatin1("Flash Video"), QString::fromLatin1(".f4v"));
    add_type(QString::fromLatin1("audio/mp4"), QString::fromLatin1("audio"), QString::fromLatin1("Flash Audio"), QString::fromLatin1(".f4a"));
    // QuickTime formats
    add_type(QString::fromLatin1("video/quicktime"), QString::fromLatin1("video"), QString::fromLatin1("QuickTime Video"), QString::fromLatin1(".qt"));
    add_type(QString::fromLatin1("video/quicktime"), QString::fromLatin1("video"), QString::fromLatin1("QuickTime Video"), QString::fromLatin1(".mov"));
    add_type(QString::fromLatin1("video/quicktime"), QString::fromLatin1("video"), QString::fromLatin1("QuickTime HD Video"), QString::fromLatin1(".hdmov"));
    // Real Media formats
    add_type(QString::fromLatin1("application/vnd.rn-realmedia"), QString::fromLatin1("video"), QString::fromLatin1("Real Media Video"), QString::fromLatin1(".rm"));
    add_type(QString::fromLatin1("application/vnd.rn-realmedia-vbr"), QString::fromLatin1("video"), QString::fromLatin1("Real Media Video"), QString::fromLatin1(".rmvb"));
    add_type(QString::fromLatin1("audio/vnd.rn-realaudio"), QString::fromLatin1("audio"), QString::fromLatin1("Real Media Audio"), QString::fromLatin1(".ra"));
    add_type(QString::fromLatin1("audio/vnd.rn-realaudio"), QString::fromLatin1("audio"), QString::fromLatin1("Real Media Audio"), QString::fromLatin1(".ram"));
    // 3GPP formats
    add_type(QString::fromLatin1("audio/3gpp"), QString::fromLatin1("audio"), QString::fromLatin1("3GPP Audio"), QString::fromLatin1(".3ga"));
    add_type(QString::fromLatin1("audio/3gpp2"), QString::fromLatin1("audio"), QString::fromLatin1("3GPP Audio"), QString::fromLatin1(".3ga2"));
    add_type(QString::fromLatin1("video/3gpp"), QString::fromLatin1("video"), QString::fromLatin1("3GPP Video"), QString::fromLatin1(".3gpp"));
    add_type(QString::fromLatin1("video/3gpp"), QString::fromLatin1("video"), QString::fromLatin1("3GPP Video"), QString::fromLatin1(".3gp"));
    add_type(QString::fromLatin1("video/3gpp2"), QString::fromLatin1("video"), QString::fromLatin1("3GPP Video"), QString::fromLatin1(".3gp2"));
    add_type(QString::fromLatin1("video/3gpp2"), QString::fromLatin1("video"), QString::fromLatin1("3GPP Video"), QString::fromLatin1(".3g2"));
    // Video game formats
    add_type(QString(), QString::fromLatin1("audio"), QString::fromLatin1("AY Audio"), QString::fromLatin1(".ay"));
    add_type(QString(), QString::fromLatin1("audio"), QString::fromLatin1("GBS Audio"), QString::fromLatin1(".gbs"));
    add_type(QString(), QString::fromLatin1("audio"), QString::fromLatin1("GYM Audio"), QString::fromLatin1(".gym"));
    add_type(QString(), QString::fromLatin1("audio"), QString::fromLatin1("HES Audio"), QString::fromLatin1(".hes"));
    add_type(QString(), QString::fromLatin1("audio"), QString::fromLatin1("KSS Audio"), QString::fromLatin1(".kss"));
    add_type(QString(), QString::fromLatin1("audio"), QString::fromLatin1("NSF Audio"), QString::fromLatin1(".nsf"));
    add_type(QString(), QString::fromLatin1("audio"), QString::fromLatin1("NSFE Audio"), QString::fromLatin1(".nsfe"));
    add_type(QString(), QString::fromLatin1("audio"), QString::fromLatin1("SAP Audio"), QString::fromLatin1(".sap"));
    add_type(QString(), QString::fromLatin1("audio"), QString::fromLatin1("SPC Audio"), QString::fromLatin1(".spc"));
    add_type(QString(), QString::fromLatin1("audio"), QString::fromLatin1("VGM Audio"), QString::fromLatin1(".vgm"));
    add_type(QString(), QString::fromLatin1("audio"), QString::fromLatin1("VGZ Audio"), QString::fromLatin1(".vgz"));
    // Playlist formats
    add_type(QString::fromLatin1("audio/x-mpegurl"), QString::fromLatin1("audio"), QString::fromLatin1("M3U Playlist"), QString::fromLatin1(".m3u"));
    add_type(QString::fromLatin1("audio/x-mpegurl"), QString::fromLatin1("audio"), QString::fromLatin1("M3U Playlist"), QString::fromLatin1(".m3u8"));
    add_type(QString::fromLatin1("audio/x-scpls"), QString::fromLatin1("audio"), QString::fromLatin1("PLS Playlist"), QString::fromLatin1(".pls"));
    add_type(QString(), QString::fromLatin1("audio"), QString::fromLatin1("CUE Sheet"), QString::fromLatin1(".cue"));

    // Register "Default Programs" entry
    const QString key6 = QString::fromLatin1("HKEY_LOCAL_MACHINE\\SOFTWARE\\RegisteredApplications");
    QSettings settings6(key6, QSettings::NativeFormat);
    if (settings6.status() != QSettings::NoError)
    {
        return false;
    }

    settings6.setValue(QString::fromLatin1("SPlayer"), QString::fromLatin1("SOFTWARE\\Clients\\Media\\SPlayer\\Capabilities"));

    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

    return true;
}
