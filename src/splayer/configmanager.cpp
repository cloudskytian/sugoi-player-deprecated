#include "bakaengine.h"
#include "util.h"
#include "ui/mainwindow.h"
#include "mpvhandler.h"
#include "ui_mainwindow.h"
#include "fileassoc.h"

#include <QDir>
#include <QSettings>

void BakaEngine::LoadSettings()
{
    QSettings settings(Util::SettingsLocation(), QSettings::IniFormat);
    window->setAlwaysCheckFileAssoc(settings.value(QString::fromLatin1("alwaysCheckFileAssoc"), true).toBool());
    QString regType = settings.value(QString::fromLatin1("fileAssoc"), QString::fromLatin1("all")).toString();
    if (regType == QString::fromLatin1("all"))
    {
        window->setFileAssocType(FileAssoc::reg_type::ALL);
    }
    else if (regType == QString::fromLatin1("video"))
    {
        window->setFileAssocType(FileAssoc::reg_type::VIDEO_ONLY);
    }
    else if (regType == QString::fromLatin1("audio"))
    {
        window->setFileAssocType(FileAssoc::reg_type::AUDIO_ONLY);
    }
    else if (regType == QString::fromLatin1("none"))
    {
        window->setFileAssocType(FileAssoc::reg_type::NONE);
    }
    window->setAllowRunInBackground(settings.value(QString::fromLatin1("allowRunInBackground"), false).toBool());
    window->setShowVideoPreview(settings.value(QString::fromLatin1("showVideoPreview"), false).toBool());
    window->setPauseWhenMinimized(settings.value(QString::fromLatin1("pauseWhenMinimized"), true).toBool());
    window->setShowFullscreenIndicator(settings.value(QString::fromLatin1("showFullscreenIndicator"), true).toBool());
    window->setOSDShowLocalTime(settings.value(QString::fromLatin1("osdShowLocalTime"), true).toBool());
    window->setOnTop(settings.value(QString::fromLatin1("onTop"), QString::fromLatin1("never")).toString());
    window->setAutoFit(settings.value(QString::fromLatin1("autoFit"), 100).toInt());
    sysTrayIcon->setVisible(settings.value(QString::fromLatin1("trayIcon"), true).toBool());
    window->setHidePopup(settings.value(QString::fromLatin1("hidePopup"), false).toBool());
    sysTrayIcon->setContextMenu(window->contextMenu);
    window->setRemaining(settings.value(QString::fromLatin1("remaining"), true).toBool());
    window->ui->splitter->setNormalPosition(settings.value(QString::fromLatin1("splitter"), window->ui->splitter->max()*1.0/8).toInt());
    window->setDebug(settings.value(QString::fromLatin1("debug"), false).toBool());
    //window->ui->hideFilesButton->setChecked(!settings.value(QString::fromLatin1("showAll"), true).toBool());
    //root["showAll"] = true;
    window->setScreenshotDialog(settings.value(QString::fromLatin1("screenshotDialog"), true).toBool());
    window->recent.clear();

    int size = settings.beginReadArray(QString::fromLatin1("recent"));
    if (size > 0)
    {
        QList<Recent> recents;
        for (int i = 0; i < size; ++i)
        {
            settings.setArrayIndex(i);
            Recent recent;
            recent.path = settings.value(QString::fromLatin1("path"), QString()).toString();
            if (recent.path.isEmpty())
            {
                continue;
            }
            recent.title = settings.value(QString::fromLatin1("title"), QString()).toString();
            recent.time = settings.value(QString::fromLatin1("time"), 0).toInt();
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

    window->setMaxRecent(settings.value(QString::fromLatin1("maxRecent"), 100).toInt());
    window->setResume(settings.value(QString::fromLatin1("resume"), true).toBool());
    window->setHideAllControls(settings.value(QString::fromLatin1("hideAllControls"), false).toBool());
    window->setLang(settings.value(QString::fromLatin1("lang"), QString::fromLatin1("auto")).toString());
    window->setLastDir(settings.value(QString::fromLatin1("lastDir"), QString::fromLatin1(".")).toString());

    window->UpdateRecentFiles();

    input = default_input;

    size = settings.beginReadArray(QString::fromLatin1("input"));
    if (size > 0)
    {
        for (int i = 0; i < size; ++i)
        {
            settings.setArrayIndex(i);
            QString key = settings.value(QString::fromLatin1("key"), QString()).toString();
            if (key.isEmpty())
            {
                continue;
            }
            input[key] = QPair<QString, QString>{
                settings.value(QString::fromLatin1("command"), QString()).toString(),
                settings.value(QString::fromLatin1("comment"), QString()).toString()
            };
        }
    }
    settings.endArray();

    window->MapShortcuts();

    settings.beginGroup(QString::fromLatin1("mpv"));
    mpv->Volume(settings.value(QString::fromLatin1("volume"), 100).toInt());
    mpv->Speed(settings.value(QString::fromLatin1("speed"), 1.0).toDouble());
    mpv->Vo(settings.value(QString::fromLatin1("vo"), QString()).toString());
    mpv->ScreenshotFormat(settings.value(QString::fromLatin1("screenshot-format"), QString::fromLatin1("png")).toString());
    mpv->ScreenshotTemplate(settings.value(QString::fromLatin1("screenshot-template"), QString::fromLatin1("screenshot%#04n")).toString());
    mpv->ScreenshotDirectory(settings.value(QString::fromLatin1("screenshot-directory"), QString::fromLatin1(".")).toString());
    mpv->MsgLevel(settings.value(QString::fromLatin1("msg-level"), QString::fromLatin1("status")).toString());

    size = settings.beginReadArray(QString::fromLatin1("options"));
    if (size > 0)
    {
        for (int i = 0; i < size; ++i)
        {
            settings.setArrayIndex(i);
            QString key = settings.value(QString::fromLatin1("key"), QString()).toString();
            if (key.isEmpty())
            {
                continue;
            }
            mpv->SetOption(key, settings.value(QString::fromLatin1("value"), QString()).toString());
        }
    }
    settings.endArray();

    settings.endGroup();
}

void BakaEngine::SaveSettings()
{
    QSettings settings(Util::SettingsLocation(), QSettings::IniFormat);
    settings.setValue(QString::fromLatin1("alwaysCheckFileAssoc"), window->getAlwaysCheckFileAssoc());
    QString regType = QString::fromLatin1("all");
    if (window->getFileAssocType() == FileAssoc::reg_type::VIDEO_ONLY)
    {
        regType = QString::fromLatin1("video");
    }
    else if (window->getFileAssocType() == FileAssoc::reg_type::AUDIO_ONLY)
    {
        regType = QString::fromLatin1("audio");
    }
    else if (window->getFileAssocType() == FileAssoc::reg_type::NONE)
    {
        regType = QString::fromLatin1("none");
    }
    settings.setValue(QString::fromLatin1("allowRunInBackground"), window->getAllowRunInBackground());
    settings.setValue(QString::fromLatin1("showVideoPreview"), window->getShowVideoPreview());
    settings.setValue(QString::fromLatin1("pauseWhenMinimized"), window->getPauseWhenMinimized());
    settings.setValue(QString::fromLatin1("showFullscreenIndicator"), window->getShowFullscreenIndicator());
    settings.setValue(QString::fromLatin1("osdShowLocalTime"), window->getOSDShowLocalTime());
    settings.setValue(QString::fromLatin1("fileAssoc"), regType);
    settings.setValue(QString::fromLatin1("onTop"), window->onTop);
    settings.setValue(QString::fromLatin1("autoFit"), window->autoFit);
    settings.setValue(QString::fromLatin1("trayIcon"), sysTrayIcon->isVisible());
    settings.setValue(QString::fromLatin1("hidePopup"), window->hidePopup);
    settings.setValue(QString::fromLatin1("remaining"), window->remaining);
    int pos = (window->ui->splitter->position() == 0 ||
                                    window->ui->splitter->position() == window->ui->splitter->max()) ?
                                    window->ui->splitter->normalPosition() :
                                    window->ui->splitter->position();
    settings.setValue(QString::fromLatin1("splitter"), pos);
    //root["showAll"] = true; //!window->ui->hideFilesButton->isChecked();
    settings.setValue(QString::fromLatin1("screenshotDialog"), window->screenshotDialog);
    settings.setValue(QString::fromLatin1("debug"), window->debug);
    settings.setValue(QString::fromLatin1("maxRecent"), window->maxRecent);
    settings.setValue(QString::fromLatin1("lang"), window->lang);
    settings.setValue(QString::fromLatin1("lastDir"), window->lastDir);
    settings.setValue(QString::fromLatin1("resume"), window->resume);
    settings.setValue(QString::fromLatin1("hideAllControls"), window->hideAllControls);

    if (!window->recent.isEmpty())
    {
        settings.beginWriteArray(QString::fromLatin1("recent"));
        for (int i = 0; i < window->recent.size(); ++i)
        {
            settings.setArrayIndex(i);
            settings.setValue(QString::fromLatin1("path"), QDir::toNativeSeparators(window->recent.at(i).path));
            settings.setValue(QString::fromLatin1("title"), window->recent.at(i).title);
            settings.setValue(QString::fromLatin1("time"), window->recent.at(i).time);
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
        settings.beginWriteArray(QString::fromLatin1("input"));
        int i = 0;
        for (auto input_iter = input_array.begin(); input_iter != input_array.end(); ++input_iter)
        {
            settings.setArrayIndex(i);
            settings.setValue(QString::fromLatin1("key"), input_iter.key());
            settings.setValue(QString::fromLatin1("command"), input_iter->first);
            settings.setValue(QString::fromLatin1("comment"), input_iter->second);
            ++i;
        }
        settings.endArray();
    }

    settings.beginGroup(QString::fromLatin1("mpv"));
    settings.setValue(QString::fromLatin1("volume"), mpv->volume);
    settings.setValue(QString::fromLatin1("speed"), mpv->speed);
    settings.setValue(QString::fromLatin1("vo"), mpv->vo);
    settings.setValue(QString::fromLatin1("screenshot-format"), mpv->screenshotFormat);
    settings.setValue(QString::fromLatin1("screenshot-template"), mpv->screenshotTemplate);
    settings.setValue(QString::fromLatin1("screenshot-directory"), QDir::toNativeSeparators(mpv->screenshotDir));
    settings.setValue(QString::fromLatin1("msg-level"), mpv->msgLevel);
    settings.endGroup();
}
