#include "sugoiengine.h"
#include "util.h"
#include "ui/mainwindow.h"
#include "mpvwidget.h"
#include "ui_mainwindow.h"
#ifdef Q_OS_WIN
#include "fileassoc.h"
#endif

#include <QDir>
#include <QSettings>

void SugoiEngine::LoadSettings()
{
    QSettings settings(Util::SettingsLocation(), QSettings::IniFormat);
#ifdef Q_OS_WIN
    window->setAlwaysCheckFileAssoc(settings.value(QStringLiteral("alwaysCheckFileAssoc"), true).toBool());
    QString regType = settings.value(QStringLiteral("fileAssoc"), QStringLiteral("all")).toString();
    if (regType == QStringLiteral("all"))
    {
        window->setFileAssocType(FileAssoc::reg_type::ALL);
    }
    else if (regType == QStringLiteral("video"))
    {
        window->setFileAssocType(FileAssoc::reg_type::VIDEO_ONLY);
    }
    else if (regType == QStringLiteral("audio"))
    {
        window->setFileAssocType(FileAssoc::reg_type::AUDIO_ONLY);
    }
    else if (regType == QStringLiteral("none"))
    {
        window->setFileAssocType(FileAssoc::reg_type::NONE);
    }
#endif
    window->setSkinFile(settings.value(QStringLiteral("skin"), QStringLiteral("Default")).toString());
    window->setAllowRunInBackground(settings.value(QStringLiteral("allowRunInBackground"), false).toBool());
    window->setPauseWhenMinimized(settings.value(QStringLiteral("pauseWhenMinimized"), true).toBool());
    window->setShowFullscreenIndicator(settings.value(QStringLiteral("showFullscreenIndicator"), true).toBool());
    window->setOSDShowLocalTime(settings.value(QStringLiteral("osdShowLocalTime"), true).toBool());
    window->setOnTop(settings.value(QStringLiteral("onTop"), QStringLiteral("never")).toString());
    window->setAutoFit(settings.value(QStringLiteral("autoFit"), 100).toInt());
    window->setHidePopup(settings.value(QStringLiteral("hidePopup"), false).toBool());
    window->setRemaining(settings.value(QStringLiteral("remaining"), true).toBool());
    window->ui->splitter->setNormalPosition(settings.value(QStringLiteral("splitter"), window->ui->splitter->max()*1.0/8).toInt());
    window->setDebug(settings.value(QStringLiteral("debug"), false).toBool());
    //window->ui->hideFilesButton->setChecked(!settings.value(QStringLiteral("showAll"), true).toBool());
    //root["showAll"] = true;
    window->setScreenshotDialog(settings.value(QStringLiteral("screenshotDialog"), true).toBool());
    window->recent.clear();

    int size = settings.beginReadArray(QStringLiteral("recent"));
    if (size > 0)
    {
        QList<Recent> recents;
        for (int i = 0; i < size; ++i)
        {
            settings.setArrayIndex(i);
            Recent recent;
            recent.path = settings.value(QStringLiteral("path"), QString()).toString();
            if (recent.path.isEmpty())
            {
                continue;
            }
            recent.title = settings.value(QStringLiteral("title"), QString()).toString();
            recent.time = settings.value(QStringLiteral("time"), 0).toInt();
            recents.append(recent);
        }
        if (recents.count() > 1)
        {
            QList<Recent> recentList(recents);
            for (int i = 0; i < (recents.count() - 1); ++i)
            {
                for (int j = (i + 1); j < recents.count(); ++j)
                {
                    if (recents.at(i).path == recents.at(j).path)
                    {
                        recentList.removeAt(i);
                    }
                }
            }
            if (!recentList.isEmpty())
            {
                window->recent.append(recentList);
            }
        }
        else if (recents.count() == 1)
        {
            window->recent.append(recents);
        }
    }
    settings.endArray();

    window->setMaxRecent(settings.value(QStringLiteral("maxRecent"), 100).toInt());
    window->setResume(settings.value(QStringLiteral("resume"), true).toBool());
    window->setHideAllControls(settings.value(QStringLiteral("hideAllControls"), false).toBool());
    window->setLang(settings.value(QStringLiteral("lang"), QStringLiteral("auto")).toString());

    window->UpdateRecentFiles();

    input = default_input;

    size = settings.beginReadArray(QStringLiteral("input"));
    if (size > 0)
    {
        for (int i = 0; i < size; ++i)
        {
            settings.setArrayIndex(i);
            QString key = settings.value(QStringLiteral("key"), QString()).toString();
            if (key.isEmpty())
            {
                continue;
            }
            input[key] = QPair<QString, QString>{
                settings.value(QStringLiteral("command"), QString()).toString(),
                settings.value(QStringLiteral("comment"), QString()).toString()
            };
        }
    }
    settings.endArray();

    window->MapShortcuts();

    settings.beginGroup(QStringLiteral("mpv"));
    mpv->fileFullPath = settings.value(QStringLiteral("lastFile"), QStringLiteral(".")).toString();
    mpv->Volume(settings.value(QStringLiteral("volume"), 100).toInt());
    mpv->Speed(settings.value(QStringLiteral("speed"), 1.0).toDouble());
    mpv->Hwdec(settings.value(QStringLiteral("hwdec"), true).toBool());
    //mpv->Vo(settings.value(QStringLiteral("vo"), QStringLiteral("opengl-cb")).toString());
    mpv->ScreenshotFormat(settings.value(QStringLiteral("screenshot-format"), QStringLiteral("png")).toString());
    mpv->ScreenshotTemplate(settings.value(QStringLiteral("screenshot-template"), QStringLiteral("screenshot%#04n")).toString());
    mpv->ScreenshotDirectory(settings.value(QStringLiteral("screenshot-directory"), Util::SnapDirLocation()).toString());
    mpv->MsgLevel(settings.value(QStringLiteral("msg-level"), QStringLiteral("status")).toString());

    size = settings.beginReadArray(QStringLiteral("options"));
    if (size > 0)
    {
        for (int i = 0; i < size; ++i)
        {
            settings.setArrayIndex(i);
            QString key = settings.value(QStringLiteral("key"), QString()).toString();
            if (key.isEmpty())
            {
                continue;
            }
            mpv->setOption(key, settings.value(QStringLiteral("value"), QString()).toString());
        }
    }
    settings.endArray();

    settings.endGroup();
}

void SugoiEngine::SaveSettings()
{
    QSettings settings(Util::SettingsLocation(), QSettings::IniFormat);
#ifdef Q_OS_WIN
    settings.setValue(QStringLiteral("alwaysCheckFileAssoc"), window->getAlwaysCheckFileAssoc());
    QString regType = QStringLiteral("all");
    if (window->getFileAssocType() == FileAssoc::reg_type::VIDEO_ONLY)
    {
        regType = QStringLiteral("video");
    }
    else if (window->getFileAssocType() == FileAssoc::reg_type::AUDIO_ONLY)
    {
        regType = QStringLiteral("audio");
    }
    else if (window->getFileAssocType() == FileAssoc::reg_type::NONE)
    {
        regType = QStringLiteral("none");
    }
#endif
    settings.setValue(QStringLiteral("skin"), window->getSkinFile());
    settings.setValue(QStringLiteral("allowRunInBackground"), window->getAllowRunInBackground());
    settings.setValue(QStringLiteral("pauseWhenMinimized"), window->getPauseWhenMinimized());
    settings.setValue(QStringLiteral("showFullscreenIndicator"), window->getShowFullscreenIndicator());
    settings.setValue(QStringLiteral("osdShowLocalTime"), window->getOSDShowLocalTime());
#ifdef Q_OS_WIN
    settings.setValue(QStringLiteral("fileAssoc"), regType);
#endif
    settings.setValue(QStringLiteral("onTop"), window->onTop);
    settings.setValue(QStringLiteral("autoFit"), window->autoFit);
    settings.setValue(QStringLiteral("hidePopup"), window->hidePopup);
    settings.setValue(QStringLiteral("remaining"), window->remaining);
    int pos = (window->ui->splitter->position() == 0 ||
                                    window->ui->splitter->position() == window->ui->splitter->max()) ?
                                    window->ui->splitter->normalPosition() :
                                    window->ui->splitter->position();
    settings.setValue(QStringLiteral("splitter"), pos);
    //root["showAll"] = true; //!window->ui->hideFilesButton->isChecked();
    settings.setValue(QStringLiteral("screenshotDialog"), window->screenshotDialog);
    settings.setValue(QStringLiteral("debug"), window->debug);
    settings.setValue(QStringLiteral("maxRecent"), window->maxRecent);
    settings.setValue(QStringLiteral("lang"), window->lang);
    settings.setValue(QStringLiteral("resume"), window->resume);
    settings.setValue(QStringLiteral("hideAllControls"), window->hideAllControls);

    if (!window->recent.isEmpty())
    {
        settings.beginWriteArray(QStringLiteral("recent"));
        for (int i = 0; i < window->recent.size(); ++i)
        {
            settings.setArrayIndex(i);
            settings.setValue(QStringLiteral("path"), QDir::toNativeSeparators(window->recent.at(i).path));
            settings.setValue(QStringLiteral("title"), window->recent.at(i).title);
            settings.setValue(QStringLiteral("time"), window->recent.at(i).time);
        }
        settings.endArray();
    }

    QHash<QString, QPair<QString, QString>> input_array;
    for(auto input_iter = input.begin(); input_iter != input.end(); ++input_iter)
    {
        auto default_iter = default_input.find(input_iter.key());
        if(default_iter != default_input.end())
        {
            if(input_iter->first == default_iter->first &&
               input_iter->second == default_iter->second) // skip entries that are the same as a default_input entry
                continue;
        }
        else // not found in defaults
        {
            if(*input_iter == QPair<QString, QString>({QString(), QString()})) // skip empty entries
                continue;
        }
        input_array[input_iter.key()] = QPair<QString, QString>{
            input_iter->first,
            input_iter->second
        };
    }
    if (!input_array.isEmpty())
    {
        settings.beginWriteArray(QStringLiteral("input"));
        int i = 0;
        for (auto input_iter = input_array.begin(); input_iter != input_array.end(); ++input_iter)
        {
            settings.setArrayIndex(i);
            settings.setValue(QStringLiteral("key"), input_iter.key());
            settings.setValue(QStringLiteral("command"), input_iter->first);
            settings.setValue(QStringLiteral("comment"), input_iter->second);
            ++i;
        }
        settings.endArray();
    }

    settings.beginGroup(QStringLiteral("mpv"));
    settings.setValue(QStringLiteral("lastFile"), QDir::toNativeSeparators(mpv->fileFullPath));
    settings.setValue(QStringLiteral("hwdec"), mpv->hwdec);
    settings.setValue(QStringLiteral("volume"), mpv->volume);
    settings.setValue(QStringLiteral("speed"), mpv->speed);
    settings.setValue(QStringLiteral("vo"), mpv->vo);
    settings.setValue(QStringLiteral("screenshot-format"), mpv->screenshotFormat);
    settings.setValue(QStringLiteral("screenshot-template"), mpv->screenshotTemplate);
    settings.setValue(QStringLiteral("screenshot-directory"), QDir::toNativeSeparators(mpv->screenshotDir));
    settings.setValue(QStringLiteral("msg-level"), mpv->msgLevel);
    settings.endGroup();
}
