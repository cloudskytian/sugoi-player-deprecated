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
#ifdef _DEBUG
    return FileAssoc::reg_state::ALL_REGISTERED;
#endif
    const QString videoKey = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\io.SugoiPlayer.avi");
    const QString audioKey = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\io.SugoiPlayer.mp3");
    QSettings videoSettings(videoKey, QSettings::NativeFormat);
    QSettings audioSettings(audioKey, QSettings::NativeFormat);
    if (videoSettings.contains(QStringLiteral("FriendlyTypeName"))
            && audioSettings.contains(QStringLiteral("FriendlyTypeName")))
    {
        return FileAssoc::reg_state::ALL_REGISTERED;
    }
     if (videoSettings.contains(QStringLiteral("FriendlyTypeName"))
             || audioSettings.contains(QStringLiteral("FriendlyTypeName")))
    {
        return FileAssoc::reg_state::SOME_REGISTERED;
    }
    return FileAssoc::reg_state::NOT_REGISTERED;
}

void FileAssoc::deleteRegistryKey(const QString &key)
{
#ifdef _DEBUG
    return;
#endif
    QSettings settings(key, QSettings::NativeFormat);
    settings.clear();
}

void FileAssoc::unregisterMediaFiles(reg_type type)
{
#ifdef _DEBUG
    return;
#endif
    if (type == FileAssoc::reg_type::NONE)
    {
        return;
    }

    const QString filePath = QCoreApplication::applicationFilePath();
    const QString fileName = QFileInfo(filePath).fileName();

    // Delete "App Paths" entry
    const QString key = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\") + fileName;
    deleteRegistryKey(key);

    // Delete HKLM subkeys
    const QString key2 = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\Applications\\") + fileName;
    deleteRegistryKey(key2);

    const QString key3 = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\SystemFileAssociations\\video\\OpenWithList\\") + fileName;
    deleteRegistryKey(key3);

    const QString key4 = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\SystemFileAssociations\\audio\\OpenWithList\\") + fileName;
    deleteRegistryKey(key4);

    // Delete "Default Programs" entry
    const QString key5 = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\RegisteredApplications");
    QSettings settings5(key5, QSettings::NativeFormat);
    settings5.remove(QStringLiteral("SugoiPlayer"));

    const QString key6 = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Clients\\Media\\SugoiPlayer\\Capabilities");
    deleteRegistryKey(key6);

    // Delete all OpenWithProgIds referencing ProgIds that start with io.SugoiPlayer.
    // TODO

    // Delete all ProgIds starting with io.SugoiPlayer.
    const QString key8 = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes");
    QSettings settings8(key8, QSettings::NativeFormat);
    QStringList key8s = settings8.allKeys();
    for (int i = 0; i <= (key8s.count() - 1); ++i)
    {
        const QString& currentKeyName = key8s.at(i);
        if (currentKeyName.startsWith(QStringLiteral("io.SugoiPlayer")))
        {
            settings8.remove(currentKeyName);
        }
    }

    ::SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

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
#ifdef _DEBUG
    return true;
#endif
    QSettings settings(key, QSettings::NativeFormat);
    if (settings.status() != QSettings::NoError)
    {
        return false;
    }

    const QString filePath = QCoreApplication::applicationFilePath();

    settings.setValue(QStringLiteral("FriendlyAppName"), QStringLiteral("SugoiPlayer"));
    settings.beginGroup(QStringLiteral("shell"));
    // Set the default verb to "play"
    settings.setValue(QStringLiteral("."), QStringLiteral("play"));
    // Hide the "open" verb from the context menu, since it's the same as "play"
    settings.beginGroup(QStringLiteral("open"));
    settings.setValue(QStringLiteral("LegacyDisable"), QString());
    settings.beginGroup(QStringLiteral("command"));
    // Set open command
    settings.setValue(QStringLiteral(".")
                      , QLatin1Char('"') + QDir::toNativeSeparators(filePath)
                      + QStringLiteral("\" \"%1\""));
    settings.endGroup();
    settings.endGroup();
    // Add "play" verb
    settings.beginGroup(QStringLiteral("play"));
    settings.setValue(QStringLiteral("."), QStringLiteral("&Play"));
    settings.beginGroup(QStringLiteral("command"));
    settings.setValue(QStringLiteral(".")
                      , QLatin1Char('"') + QDir::toNativeSeparators(filePath)
                      + QStringLiteral("\" \"%1\""));
    settings.endGroup();
    settings.endGroup();
    settings.endGroup();

    return true;
}

bool FileAssoc::add_progid(const QString &prog_id, const QString &friendly_name, const QString &icon_path)
{
#ifdef _DEBUG
    return true;
#endif
    // Add ProgId, edit flags are FTA_OpenIsSafe | FTA_AlwaysUseDirectInvoke
    const QString key = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\") + prog_id;
    QSettings settings(key, QSettings::NativeFormat);
    if (settings.status() != QSettings::NoError)
    {
        return false;
    }

    settings.setValue(QStringLiteral("."), friendly_name);
    settings.setValue(QStringLiteral("EditFlags"), 4259840);
    settings.setValue(QStringLiteral("FriendlyTypeName"), friendly_name);
    settings.beginGroup(QStringLiteral("DefaultIcon"));
    settings.setValue(QStringLiteral("."), QDir::toNativeSeparators(icon_path));
    settings.endGroup();

    return add_verbs(key);
}

bool FileAssoc::update_extension(const QString &extension, const QString &prog_id, const QString &mime_type, const QString &perceived_type)
{
#ifdef _DEBUG
    return true;
#endif
    // Add information about the file extension
    const QString key = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\") + extension;
    QSettings settings(key, QSettings::NativeFormat);
    if (settings.status() != QSettings::NoError)
    {
        return false;
    }

    settings.setValue(QStringLiteral("Content Type"), mime_type);
    settings.setValue(QStringLiteral("PerceivedType"), perceived_type);
    settings.beginGroup(QStringLiteral("OpenWithProgIds"));
    settings.setValue(prog_id, QString());
    settings.endGroup();

    // Add type to SupportedTypes
    const QString filePath = QCoreApplication::applicationFilePath();
    const QString fileName = QFileInfo(filePath).fileName();
    const QString key2 = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\Applications\\") + fileName + QStringLiteral("\\SupportedTypes");
    QSettings settings2(key2, QSettings::NativeFormat);
    if (settings2.status() != QSettings::NoError)
    {
        return false;
    }

    settings2.setValue(extension, QString());

    // Add type to the Default Programs control panel
    const QString key3 = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Clients\\Media\\SugoiPlayer\\Capabilities\\FileAssociations");
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
#ifdef _DEBUG
    return true;
#endif
    if (regType == FileAssoc::reg_type::NONE)
    {
        return false;
    }
    if (regType == FileAssoc::reg_type::VIDEO_ONLY)
    {
        if (perceived_type == QStringLiteral("audio"))
        {
            return false;
        }
    }
    if (regType == FileAssoc::reg_type::AUDIO_ONLY)
    {
        if (perceived_type == QStringLiteral("video"))
        {
            return false;
        }
    }
    // Add ProgId
    const QString prog_id = QStringLiteral("io.SugoiPlayer") + extension;
    QString iconPath = QLatin1Char('"') + QCoreApplication::applicationFilePath() + QStringLiteral("\",0");
    const QString iconLibPath = QCoreApplication::applicationDirPath() + QLatin1Char('/') + QStringLiteral("iconlib.dll");
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
            str = str.remove(QStringLiteral("\""));
            iconPath = QLatin1Char('"') + iconLibPath + QStringLiteral("\",") + str;
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
#ifdef _DEBUG
    return true;
#endif
    regType = type;

    if (regType == FileAssoc::reg_type::NONE)
    {
        unregisterMediaFiles(FileAssoc::reg_type::ALL);
        return true;
    }

    const QString filePath = QCoreApplication::applicationFilePath();
    const QString fileName = QFileInfo(filePath).fileName();

    // Register Sugoi.exe under the "App Paths" key, so it can be found by
    // ShellExecute, the run command, the start menu, etc.
    const QString key1 = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\") + fileName;
    QSettings settings1(key1, QSettings::NativeFormat);
    if (settings1.status() != QSettings::NoError)
    {
        return false;
    }

    settings1.setValue(QStringLiteral("."), QDir::toNativeSeparators(filePath));
    settings1.setValue(QStringLiteral("UseUrl"), 1);

    // Register Sugoi.exe under the "Applications" key to add some default verbs for
    // when SugoiPlayer is used from the "Open with" menu
    const QString key2 = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\Applications\\") + fileName;
    if (!add_verbs(key2))
    {
        return false;
    }

    // Add SugoiPlayer to the "Open with" list for all video and audio file types
    if (regType != FileAssoc::reg_type::AUDIO_ONLY)
    {
        const QString key3 = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\SystemFileAssociations\\video\\OpenWithList\\") + fileName;
        QSettings settings3(key3, QSettings::NativeFormat);
        if (settings3.status() != QSettings::NoError)
        {
            return false;
        }

        settings3.setValue(QStringLiteral("FileName"), fileName);
    }

    if (regType != FileAssoc::reg_type::VIDEO_ONLY)
    {
        const QString key4 = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\SystemFileAssociations\\audio\\OpenWithList\\") + fileName;
        QSettings settings4(key4, QSettings::NativeFormat);
        if (settings4.status() != QSettings::NoError)
        {
            return false;
        }

        settings4.setValue(QStringLiteral("FileName"), fileName);
    }

    // Add a capabilities key for SugoiPlayer, which is registered later on for use in the
    // "Default Programs" control panel
    const QString key5 = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Clients\\Media\\SugoiPlayer\\Capabilities");
    QSettings settings5(key5, QSettings::NativeFormat);
    if (settings5.status() != QSettings::NoError)
    {
        return false;
    }

    settings5.setValue(QStringLiteral("ApplicationName"), QStringLiteral("SugoiPlayer"));
    settings5.setValue(QStringLiteral("ApplicationDescription"), QStringLiteral("SugoiPlayer, a multimedia player for Windows 7+ based on libmpv and Qt."));

    // Add file types
    // DVD/Blu-ray audio formats
    add_type(QStringLiteral("audio/ac3"), QStringLiteral("audio"), QStringLiteral("AC-3 Audio"), QStringLiteral(".ac3"));
    add_type(QStringLiteral("audio/ac3"), QStringLiteral("audio"), QStringLiteral("AC-3 Audio"), QStringLiteral(".a52"));
    add_type(QStringLiteral("audio/eac3"), QStringLiteral("audio"), QStringLiteral("E-AC-3 Audio"), QStringLiteral(".eac3"));
    add_type(QStringLiteral("audio/vnd.dolby.mlp"), QStringLiteral("audio"), QStringLiteral("MLP Audio"), QStringLiteral(".mlp"));
    add_type(QStringLiteral("audio/vnd.dts"), QStringLiteral("audio"), QStringLiteral("DTS Audio"), QStringLiteral(".dts"));
    add_type(QStringLiteral("audio/vnd.dts.hd"), QStringLiteral("audio"), QStringLiteral("DTS-HD Audio"), QStringLiteral(".dts-hd"));
    add_type(QStringLiteral("audio/vnd.dts.hd"), QStringLiteral("audio"), QStringLiteral("DTS-HD Audio"), QStringLiteral(".dtshd"));
    add_type(QString(), QStringLiteral("audio"), QStringLiteral("TrueHD Audio"), QStringLiteral(".true-hd"));
    add_type(QString(), QStringLiteral("audio"), QStringLiteral("TrueHD Audio"), QStringLiteral(".thd"));
    add_type(QString(), QStringLiteral("audio"), QStringLiteral("TrueHD Audio"), QStringLiteral(".truehd"));
    add_type(QString(), QStringLiteral("audio"), QStringLiteral("TrueHD Audio"), QStringLiteral(".thd+ac3"));
    add_type(QString(), QStringLiteral("audio"), QStringLiteral("True Audio"), QStringLiteral(".tta"));
    // Uncompressed formats
    add_type(QString(), QStringLiteral("audio"), QStringLiteral("PCM Audio"), QStringLiteral(".pcm"));
    add_type(QStringLiteral("audio/wav"), QStringLiteral("audio"), QStringLiteral("Wave Audio"), QStringLiteral(".wav"));
    add_type(QStringLiteral("audio/aiff"), QStringLiteral("audio"), QStringLiteral("AIFF Audio"), QStringLiteral(".aiff"));
    add_type(QStringLiteral("audio/aiff"), QStringLiteral("audio"), QStringLiteral("AIFF Audio"), QStringLiteral(".aif"));
    add_type(QStringLiteral("audio/aiff"), QStringLiteral("audio"), QStringLiteral("AIFF Audio"), QStringLiteral(".aifc"));
    add_type(QStringLiteral("audio/amr"), QStringLiteral("audio"), QStringLiteral("AMR Audio"), QStringLiteral(".amr"));
    add_type(QStringLiteral("audio/amr-wb"), QStringLiteral("audio"), QStringLiteral("AMR-WB Audio"), QStringLiteral(".awb"));
    add_type(QStringLiteral("audio/basic"), QStringLiteral("audio"), QStringLiteral("AU Audio"), QStringLiteral(".au"));
    add_type(QStringLiteral("audio/basic"), QStringLiteral("audio"), QStringLiteral("AU Audio"), QStringLiteral(".snd"));
    add_type(QString(), QStringLiteral("audio"), QStringLiteral("Linear PCM Audio"), QStringLiteral(".lpcm"));
    add_type(QString(), QStringLiteral("video"), QStringLiteral("Raw YUV Video"), QStringLiteral(".yuv"));
    add_type(QString(), QStringLiteral("video"), QStringLiteral("YUV4MPEG2 Video"), QStringLiteral(".y4m"));
    // Free lossless formats
    add_type(QStringLiteral("audio/x-ape"), QStringLiteral("audio"), QStringLiteral("Monkey's Audio"), QStringLiteral(".ape"));
    add_type(QStringLiteral("audio/x-wavpack"), QStringLiteral("audio"), QStringLiteral("WavPack Audio"), QStringLiteral(".wv"));
    add_type(QStringLiteral("audio/x-shorten"), QStringLiteral("audio"), QStringLiteral("Shorten Audio"), QStringLiteral(".shn"));
    // MPEG formats
    add_type(QStringLiteral("video/vnd.dlna.mpeg-tts"), QStringLiteral("video"), QStringLiteral("MPEG-2 Transport Stream"), QStringLiteral(".m2ts"));
    add_type(QStringLiteral("video/vnd.dlna.mpeg-tts"), QStringLiteral("video"), QStringLiteral("MPEG-2 Transport Stream"), QStringLiteral(".m2t"));
    add_type(QStringLiteral("video/vnd.dlna.mpeg-tts"), QStringLiteral("video"), QStringLiteral("MPEG-2 Transport Stream"), QStringLiteral(".mts"));
    add_type(QStringLiteral("video/vnd.dlna.mpeg-tts"), QStringLiteral("video"), QStringLiteral("MPEG-2 Transport Stream"), QStringLiteral(".mtv"));
    add_type(QStringLiteral("video/vnd.dlna.mpeg-tts"), QStringLiteral("video"), QStringLiteral("MPEG-2 Transport Stream"), QStringLiteral(".ts"));
    add_type(QStringLiteral("video/vnd.dlna.mpeg-tts"), QStringLiteral("video"), QStringLiteral("MPEG-2 Transport Stream"), QStringLiteral(".tsv"));
    add_type(QStringLiteral("video/vnd.dlna.mpeg-tts"), QStringLiteral("video"), QStringLiteral("MPEG-2 Transport Stream"), QStringLiteral(".tsa"));
    add_type(QStringLiteral("video/vnd.dlna.mpeg-tts"), QStringLiteral("video"), QStringLiteral("MPEG-2 Transport Stream"), QStringLiteral(".tts"));
    add_type(QStringLiteral("video/vnd.dlna.mpeg-tts"), QStringLiteral("video"), QStringLiteral("MPEG-2 Transport Stream"), QStringLiteral(".trp"));
    add_type(QStringLiteral("audio/vnd.dlna.adts"), QStringLiteral("audio"), QStringLiteral("ADTS Audio"), QStringLiteral(".adts"));
    add_type(QStringLiteral("audio/vnd.dlna.adts"), QStringLiteral("audio"), QStringLiteral("ADTS Audio"), QStringLiteral(".adt"));
    add_type(QStringLiteral("audio/mpeg"), QStringLiteral("audio"), QStringLiteral("MPEG Audio"), QStringLiteral(".mpa"));
    add_type(QStringLiteral("audio/mpeg"), QStringLiteral("audio"), QStringLiteral("MPEG Audio"), QStringLiteral(".m1a"));
    add_type(QStringLiteral("audio/mpeg"), QStringLiteral("audio"), QStringLiteral("MPEG Audio"), QStringLiteral(".m2a"));
    add_type(QStringLiteral("audio/mpeg"), QStringLiteral("audio"), QStringLiteral("MPEG Audio"), QStringLiteral(".mp1"));
    add_type(QStringLiteral("audio/mpeg"), QStringLiteral("audio"), QStringLiteral("MPEG Audio"), QStringLiteral(".mp2"));
    add_type(QStringLiteral("audio/mpeg"), QStringLiteral("audio"), QStringLiteral("MP3 Audio"), QStringLiteral(".mp3"));
    add_type(QStringLiteral("video/mpeg"), QStringLiteral("video"), QStringLiteral("MPEG Video"), QStringLiteral(".mpeg"));
    add_type(QStringLiteral("video/mpeg"), QStringLiteral("video"), QStringLiteral("MPEG Video"), QStringLiteral(".mpg"));
    add_type(QStringLiteral("video/mpeg"), QStringLiteral("video"), QStringLiteral("MPEG Video"), QStringLiteral(".mpe"));
    add_type(QStringLiteral("video/mpeg"), QStringLiteral("video"), QStringLiteral("MPEG Video"), QStringLiteral(".mpeg2"));
    add_type(QStringLiteral("video/mpeg"), QStringLiteral("video"), QStringLiteral("MPEG Video"), QStringLiteral(".m1v"));
    add_type(QStringLiteral("video/mpeg"), QStringLiteral("video"), QStringLiteral("MPEG Video"), QStringLiteral(".m2v"));
    add_type(QStringLiteral("video/mpeg"), QStringLiteral("video"), QStringLiteral("MPEG Video"), QStringLiteral(".mp2v"));
    add_type(QStringLiteral("video/mpeg"), QStringLiteral("video"), QStringLiteral("MPEG Video"), QStringLiteral(".mpv"));
    add_type(QStringLiteral("video/mpeg"), QStringLiteral("video"), QStringLiteral("MPEG Video"), QStringLiteral(".mpv2"));
    add_type(QStringLiteral("video/mpeg"), QStringLiteral("video"), QStringLiteral("MPEG Video"), QStringLiteral(".mod"));
    add_type(QStringLiteral("video/mpeg"), QStringLiteral("video"), QStringLiteral("MPEG Video"), QStringLiteral(".tod"));
    add_type(QStringLiteral("video/dvd"), QStringLiteral("video"), QStringLiteral("Video Object"), QStringLiteral(".vob"));
    add_type(QStringLiteral("video/dvd"), QStringLiteral("video"), QStringLiteral("Video Object"), QStringLiteral(".vro"));
    add_type(QString(), QStringLiteral("video"), QStringLiteral("Enhanced VOB"), QStringLiteral(".evob"));
    add_type(QString(), QStringLiteral("video"), QStringLiteral("Enhanced VOB"), QStringLiteral(".evo"));
    add_type(QStringLiteral("video/mp4"), QStringLiteral("video"), QStringLiteral("MPEG-4 Video"), QStringLiteral(".mpeg4"));
    add_type(QStringLiteral("video/mp4"), QStringLiteral("video"), QStringLiteral("MPEG-4 Video"), QStringLiteral(".m4v"));
    add_type(QStringLiteral("video/mp4"), QStringLiteral("video"), QStringLiteral("MPEG-4 Video"), QStringLiteral(".mp4"));
    add_type(QStringLiteral("video/mp4"), QStringLiteral("video"), QStringLiteral("MPEG-4 Video"), QStringLiteral(".mp4v"));
    add_type(QStringLiteral("video/mp4"), QStringLiteral("video"), QStringLiteral("MPEG-4 Video"), QStringLiteral(".mpg4"));
    add_type(QStringLiteral("audio/mp4"), QStringLiteral("audio"), QStringLiteral("MPEG-4 Audio"), QStringLiteral(".m4a"));
    add_type(QStringLiteral("audio/aac"), QStringLiteral("audio"), QStringLiteral("Raw AAC Audio"), QStringLiteral(".aac"));
    add_type(QString(), QStringLiteral("video"), QStringLiteral("Raw H.264/AVC Video"), QStringLiteral(".h264"));
    add_type(QString(), QStringLiteral("video"), QStringLiteral("Raw H.264/AVC Video"), QStringLiteral(".avc"));
    add_type(QString(), QStringLiteral("video"), QStringLiteral("Raw H.264/AVC Video"), QStringLiteral(".x264"));
    add_type(QString(), QStringLiteral("video"), QStringLiteral("Raw H.264/AVC Video"), QStringLiteral(".264"));
    add_type(QString(), QStringLiteral("video"), QStringLiteral("Raw H.265/HEVC Video"), QStringLiteral(".hevc"));
    add_type(QString(), QStringLiteral("video"), QStringLiteral("Raw H.265/HEVC Video"), QStringLiteral(".h265"));
    add_type(QString(), QStringLiteral("video"), QStringLiteral("Raw H.265/HEVC Video"), QStringLiteral(".x265"));
    add_type(QString(), QStringLiteral("video"), QStringLiteral("Raw H.265/HEVC Video"), QStringLiteral(".265"));
    // Xiph formats
    add_type(QStringLiteral("audio/flac"), QStringLiteral("audio"), QStringLiteral("FLAC Audio"), QStringLiteral(".flac"));
    add_type(QStringLiteral("audio/ogg"), QStringLiteral("audio"), QStringLiteral("Ogg Audio"), QStringLiteral(".oga"));
    add_type(QStringLiteral("audio/ogg"), QStringLiteral("audio"), QStringLiteral("Ogg Audio"), QStringLiteral(".ogg"));
    add_type(QStringLiteral("audio/ogg"), QStringLiteral("audio"), QStringLiteral("Opus Audio"), QStringLiteral(".opus"));
    add_type(QStringLiteral("audio/ogg"), QStringLiteral("audio"), QStringLiteral("Speex Audio"), QStringLiteral(".spx"));
    add_type(QStringLiteral("video/ogg"), QStringLiteral("video"), QStringLiteral("Ogg Video"), QStringLiteral(".ogv"));
    add_type(QStringLiteral("video/ogg"), QStringLiteral("video"), QStringLiteral("Ogg Video"), QStringLiteral(".ogm"));
    add_type(QStringLiteral("application/ogg"), QStringLiteral("video"), QStringLiteral("Ogg Video"), QStringLiteral(".ogx"));
    // Matroska formats
    add_type(QStringLiteral("video/x-matroska"), QStringLiteral("video"), QStringLiteral("Matroska Video"), QStringLiteral(".mkv"));
    add_type(QStringLiteral("video/x-matroska"), QStringLiteral("video"), QStringLiteral("Matroska 3D Video"), QStringLiteral(".mk3d"));
    add_type(QStringLiteral("audio/x-matroska"), QStringLiteral("audio"), QStringLiteral("Matroska Audio"), QStringLiteral(".mka"));
    add_type(QStringLiteral("video/webm"), QStringLiteral("video"), QStringLiteral("WebM Video"), QStringLiteral(".webm"));
    add_type(QStringLiteral("audio/webm"), QStringLiteral("audio"), QStringLiteral("WebM Audio"), QStringLiteral(".weba"));
    // Misc formats
    add_type(QStringLiteral("video/avi"), QStringLiteral("video"), QStringLiteral("Video Clip"), QStringLiteral(".avi"));
    add_type(QStringLiteral("video/avi"), QStringLiteral("video"), QStringLiteral("Video Clip"), QStringLiteral(".vfw"));
    add_type(QString(), QStringLiteral("video"), QStringLiteral("DivX Video"), QStringLiteral(".divx"));
    add_type(QString(), QStringLiteral("video"), QStringLiteral("3ivx Video"), QStringLiteral(".3iv"));
    add_type(QString(), QStringLiteral("video"), QStringLiteral("XVID Video"), QStringLiteral(".xvid"));
    add_type(QString(), QStringLiteral("video"), QStringLiteral("NUT Video"), QStringLiteral(".nut"));
    add_type(QStringLiteral("video/flc"), QStringLiteral("video"), QStringLiteral("FLIC Video"), QStringLiteral(".flic"));
    add_type(QStringLiteral("video/flc"), QStringLiteral("video"), QStringLiteral("FLIC Video"), QStringLiteral(".fli"));
    add_type(QStringLiteral("video/flc"), QStringLiteral("video"), QStringLiteral("FLIC Video"), QStringLiteral(".flc"));
    add_type(QString(), QStringLiteral("video"), QStringLiteral("Nullsoft Streaming Video"), QStringLiteral(".nsv"));
    add_type(QStringLiteral("application/gxf"), QStringLiteral("video"), QStringLiteral("General Exchange Format"), QStringLiteral(".gxf"));
    add_type(QStringLiteral("application/mxf"), QStringLiteral("video"), QStringLiteral("Material Exchange Format"), QStringLiteral(".mxf"));
    // Windows Media formats
    add_type(QStringLiteral("audio/x-ms-wma"), QStringLiteral("audio"), QStringLiteral("Windows Media Audio"), QStringLiteral(".wma"));
    add_type(QStringLiteral("video/x-ms-wm"), QStringLiteral("video"), QStringLiteral("Windows Media Video"), QStringLiteral(".wm"));
    add_type(QStringLiteral("video/x-ms-wmv"), QStringLiteral("video"), QStringLiteral("Windows Media Video"), QStringLiteral(".wmv"));
    add_type(QStringLiteral("video/x-ms-asf"), QStringLiteral("video"), QStringLiteral("Windows Media Video"), QStringLiteral(".asf"));
    add_type(QString(), QStringLiteral("video"), QStringLiteral("Microsoft Recorded TV Show"), QStringLiteral(".dvr-ms"));
    add_type(QString(), QStringLiteral("video"), QStringLiteral("Microsoft Recorded TV Show"), QStringLiteral(".dvr"));
    add_type(QString(), QStringLiteral("video"), QStringLiteral("Windows Recorded TV Show"), QStringLiteral(".wtv"));
    // DV formats
    add_type(QString(), QStringLiteral("video"), QStringLiteral("DV Video"), QStringLiteral(".dv"));
    add_type(QString(), QStringLiteral("video"), QStringLiteral("DV Video"), QStringLiteral(".hdv"));
    // Flash Video formats
    add_type(QStringLiteral("video/x-flv"), QStringLiteral("video"), QStringLiteral("Flash Video"), QStringLiteral(".flv"));
    add_type(QStringLiteral("video/mp4"), QStringLiteral("video"), QStringLiteral("Flash Video"), QStringLiteral(".f4v"));
    add_type(QStringLiteral("audio/mp4"), QStringLiteral("audio"), QStringLiteral("Flash Audio"), QStringLiteral(".f4a"));
    // QuickTime formats
    add_type(QStringLiteral("video/quicktime"), QStringLiteral("video"), QStringLiteral("QuickTime Video"), QStringLiteral(".qt"));
    add_type(QStringLiteral("video/quicktime"), QStringLiteral("video"), QStringLiteral("QuickTime Video"), QStringLiteral(".mov"));
    add_type(QStringLiteral("video/quicktime"), QStringLiteral("video"), QStringLiteral("QuickTime HD Video"), QStringLiteral(".hdmov"));
    // Real Media formats
    add_type(QStringLiteral("application/vnd.rn-realmedia"), QStringLiteral("video"), QStringLiteral("Real Media Video"), QStringLiteral(".rm"));
    add_type(QStringLiteral("application/vnd.rn-realmedia-vbr"), QStringLiteral("video"), QStringLiteral("Real Media Video"), QStringLiteral(".rmvb"));
    add_type(QStringLiteral("audio/vnd.rn-realaudio"), QStringLiteral("audio"), QStringLiteral("Real Media Audio"), QStringLiteral(".ra"));
    add_type(QStringLiteral("audio/vnd.rn-realaudio"), QStringLiteral("audio"), QStringLiteral("Real Media Audio"), QStringLiteral(".ram"));
    // 3GPP formats
    add_type(QStringLiteral("audio/3gpp"), QStringLiteral("audio"), QStringLiteral("3GPP Audio"), QStringLiteral(".3ga"));
    add_type(QStringLiteral("audio/3gpp2"), QStringLiteral("audio"), QStringLiteral("3GPP Audio"), QStringLiteral(".3ga2"));
    add_type(QStringLiteral("video/3gpp"), QStringLiteral("video"), QStringLiteral("3GPP Video"), QStringLiteral(".3gpp"));
    add_type(QStringLiteral("video/3gpp"), QStringLiteral("video"), QStringLiteral("3GPP Video"), QStringLiteral(".3gp"));
    add_type(QStringLiteral("video/3gpp2"), QStringLiteral("video"), QStringLiteral("3GPP Video"), QStringLiteral(".3gp2"));
    add_type(QStringLiteral("video/3gpp2"), QStringLiteral("video"), QStringLiteral("3GPP Video"), QStringLiteral(".3g2"));
    // Video game formats
    add_type(QString(), QStringLiteral("audio"), QStringLiteral("AY Audio"), QStringLiteral(".ay"));
    add_type(QString(), QStringLiteral("audio"), QStringLiteral("GBS Audio"), QStringLiteral(".gbs"));
    add_type(QString(), QStringLiteral("audio"), QStringLiteral("GYM Audio"), QStringLiteral(".gym"));
    add_type(QString(), QStringLiteral("audio"), QStringLiteral("HES Audio"), QStringLiteral(".hes"));
    add_type(QString(), QStringLiteral("audio"), QStringLiteral("KSS Audio"), QStringLiteral(".kss"));
    add_type(QString(), QStringLiteral("audio"), QStringLiteral("NSF Audio"), QStringLiteral(".nsf"));
    add_type(QString(), QStringLiteral("audio"), QStringLiteral("NSFE Audio"), QStringLiteral(".nsfe"));
    add_type(QString(), QStringLiteral("audio"), QStringLiteral("SAP Audio"), QStringLiteral(".sap"));
    add_type(QString(), QStringLiteral("audio"), QStringLiteral("SPC Audio"), QStringLiteral(".spc"));
    add_type(QString(), QStringLiteral("audio"), QStringLiteral("VGM Audio"), QStringLiteral(".vgm"));
    add_type(QString(), QStringLiteral("audio"), QStringLiteral("VGZ Audio"), QStringLiteral(".vgz"));
    // Playlist formats
    add_type(QStringLiteral("audio/x-mpegurl"), QStringLiteral("audio"), QStringLiteral("M3U Playlist"), QStringLiteral(".m3u"));
    add_type(QStringLiteral("audio/x-mpegurl"), QStringLiteral("audio"), QStringLiteral("M3U Playlist"), QStringLiteral(".m3u8"));
    add_type(QStringLiteral("audio/x-scpls"), QStringLiteral("audio"), QStringLiteral("PLS Playlist"), QStringLiteral(".pls"));
    add_type(QString(), QStringLiteral("audio"), QStringLiteral("CUE Sheet"), QStringLiteral(".cue"));

    // Register "Default Programs" entry
    const QString key6 = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\RegisteredApplications");
    QSettings settings6(key6, QSettings::NativeFormat);
    if (settings6.status() != QSettings::NoError)
    {
        return false;
    }

    settings6.setValue(QStringLiteral("SugoiPlayer"), QStringLiteral("SOFTWARE\\Clients\\Media\\SugoiPlayer\\Capabilities"));

    ::SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

    return true;
}
